#ifndef SEGMENTED_POOL_H
#define SEGMENTED_POOL_H

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <type_traits>

// Fixed bucket size for all pools; capacity is buckets * this.
inline constexpr size_t kSegmentBucketSize = 64;

// Lock-free segmented pool: fixed 64-slot buckets published on
// first touch, atomic high-water length, snapshot iterators.
template <typename T, size_t MaxSegs>
requires std::is_trivially_copyable_v<T>
class SegmentedPool {
  public:
    static constexpr size_t kBucketSize = kSegmentBucketSize;
    static constexpr size_t kBucketBits = 6;
    static_assert(kBucketSize == size_t{1} << kBucketBits, "bits match size");
    static_assert(MaxSegs > 0, "pool needs at least one bucket");

    static constexpr size_t capacity() { return MaxSegs * kBucketSize; }

    SegmentedPool() {
        for (auto &b : buckets_)
            b.store(nullptr, std::memory_order_relaxed);
    }
    // Single-threaded teardown: no other thread may access the pool.
    ~SegmentedPool() {
        for (auto &b : buckets_)
            delete b.load(std::memory_order_relaxed);
    }
    SegmentedPool(const SegmentedPool &) = delete;
    SegmentedPool &operator=(const SegmentedPool &) = delete;

    size_t size() const { return size_.load(std::memory_order_acquire); }
    bool empty() const { return size() == 0; }

    // Reserve the next index; nullopt when full. Publish with slot()
    // or publish(); the slot reads default until then, like a fresh push.
    std::optional<size_t> reserve() {
        size_t i = size_.fetch_add(1, std::memory_order_relaxed);
        if (i >= capacity()) {
            // Paired with our own increment; size stays above live slots.
            size_.fetch_sub(1, std::memory_order_relaxed);
            return std::nullopt;
        }
        return i;
    }

    // Reserve a slot and publish v; nullopt when full.
    std::optional<size_t> push_back(const T &v) {
        auto i = reserve();
        if (!i)
            return std::nullopt;
        slot(*i).store(v, std::memory_order_release);
        return i;
    }

    // Store v at i and cover it with the length; false over capacity.
    bool publish(size_t i, const T &v) {
        if (i >= capacity())
            return false;
        slot(i).store(v, std::memory_order_release);
        return extend_to(i + 1);
    }

    // Lower the length to n; slots stay allocated for later regrowth.
    void shrink_to(size_t n) {
        size_t cur = size_.load(std::memory_order_relaxed);
        while (cur > n &&
               !size_.compare_exchange_weak(cur, n, std::memory_order_relaxed))
            ;
    }

    // Raise the length to n without writing slots; false over capacity.
    bool extend_to(size_t n) {
        if (n > capacity())
            return false;
        size_t cur = size_.load(std::memory_order_relaxed);
        while (cur < n &&
               !size_.compare_exchange_weak(cur, n, std::memory_order_relaxed))
            ;
        return true;
    }

    // Load the value at i; writes must use slot() or publish().
    T operator[](size_t i) const { return load(i); }

    // Raw slot for CAS-claim loops; publishes its bucket on first touch.
    std::atomic<T> &slot(size_t i) {
        Bucket *b = bucket_for(i >> kBucketBits);
        return b->slots[i & (kBucketSize - 1)];
    }

    // Live atomic slots for CAS-claim loops (see find_new_id).
    auto slots() {
        return std::views::iota(size_t{0}, std::min(size(), capacity())) |
               std::views::transform(
                   [this](size_t i) -> std::atomic<T> & { return slot(i); });
    }

    // Visit every slot in published buckets, past shrinks; for teardown.
    template <typename F> void for_each_published(F &&f) {
        for (size_t b = 0; b < MaxSegs; ++b) {
            Bucket *bk = buckets_[b].load(std::memory_order_relaxed);
            if (!bk)
                continue;
            for (auto &s : bk->slots)
                f(s);
        }
    }

    // Single-threaded reset: blank all published slots, drop the length.
    void clear() {
        for_each_published(
            [](std::atomic<T> &s) { s.store(T{}, std::memory_order_relaxed); });
        size_.store(0, std::memory_order_relaxed);
    }

    class iterator {
      public:
        using value_type = T;
        using difference_type = ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        iterator() = default;
        // Multi-pass over a length snapshot; each deref reloads the slot.
        T operator*() const { return pool_->load(i_); }
        iterator &operator++() {
            ++i_;
            return *this;
        }
        iterator operator++(int) {
            iterator c = *this;
            ++i_;
            return c;
        }
        bool operator==(const iterator &o) const { return i_ == o.i_; }

      private:
        friend class SegmentedPool;
        iterator(const SegmentedPool *p, size_t i) : pool_(p), i_(i) {}
        const SegmentedPool *pool_;
        size_t i_;
    };

    iterator begin() const { return {this, 0}; }
    iterator end() const {
        // Clamp: a failed push_back can transiently overshoot capacity.
        return {this, std::min(size(), capacity())};
    }

  private:
    struct Bucket {
        std::array<std::atomic<T>, kBucketSize> slots;
        // Atomics don't self-init; blank before release publication.
        Bucket() {
            for (auto &s : slots)
                s.store(T{}, std::memory_order_relaxed);
        }
    };

    Bucket *bucket_for(size_t b) const {
        Bucket *found = buckets_[b].load(std::memory_order_acquire);
        if (found)
            return found;
        Bucket *fresh = new Bucket();
        Bucket *want = nullptr;
        if (buckets_[b].compare_exchange_strong(want, fresh,
                                                std::memory_order_acq_rel,
                                                std::memory_order_acquire))
            return fresh;
        delete fresh;
        return want;
    }

    T load(size_t i) const {
        Bucket *b = bucket_for(i >> kBucketBits);
        return b->slots[i & (kBucketSize - 1)].load(std::memory_order_acquire);
    }

    // Mutable: first touch publishes buckets even through const loads.
    mutable std::array<std::atomic<Bucket *>, MaxSegs> buckets_;
    std::atomic<size_t> size_{0};
};

#endif
