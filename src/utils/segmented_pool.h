#ifndef SEGMENTED_POOL_H
#define SEGMENTED_POOL_H

#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ranges>
#include <type_traits>

// Fixed bucket size for all pools; capacity is buckets * this.
inline constexpr size_t kSegmentBucketSize = 64;

// Objects recyclable through obtain(): default-constructible with
// an id plus atomic enabled/allocated flags.
template <typename U>
concept ReusableObject = requires(U *u, bool expected) {
    requires std::default_initializable<U>;
    u->id = 0;
    u->enabled = false;
    { u->enabled.load() } -> std::convertible_to<bool>;
    {
        u->allocated.compare_exchange_strong(expected, true)
    } -> std::same_as<bool>;
};

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
        size_t index = size_.fetch_add(1, std::memory_order_relaxed);
        if (index >= capacity()) {
            // Paired with our own increment; size stays above live slots.
            size_.fetch_sub(1, std::memory_order_relaxed);
            return std::nullopt;
        }
        return index;
    }

    // Reserve a slot and publish v; nullopt when full.
    std::optional<size_t> push_back(const T &v) {
        auto index = reserve();
        if (!index)
            return std::nullopt;
        slot(*index).store(v, std::memory_order_release);
        return index;
    }

    // Store v at index and cover it with the length; false over capacity.
    bool publish(size_t index, const T &v) {
        if (index >= capacity())
            return false;
        slot(index).store(v, std::memory_order_release);
        return extend_to(index + 1);
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

    // Claim a free object, reusing a released slot when possible;
    // nullptr when full. The common entry point for adding objects.
    T obtain()
    requires ReusableObject<std::remove_pointer_t<T>>
    {
        using U = std::remove_pointer_t<T>;
        auto scan = [&](size_t n) -> U * {
            for (size_t index = 0; index < n; ++index) {
                U *cur = slot(index).load();
                if (!cur) {
                    U *fresh = new U();
                    fresh->allocated = true;
                    fresh->enabled = false;
                    fresh->id = (int)index;
                    if (slot(index).compare_exchange_strong(cur, fresh))
                        return fresh;
                    delete fresh;
                    cur = slot(index).load();
                    if (!cur)
                        continue;
                }
                bool expected = false;
                if (!cur->enabled.load() &&
                    cur->allocated.compare_exchange_strong(expected, true)) {
                    // id is set once at allocation; rewriting it here races
                    // with unlocked x->id readers (TSAN).
                    cur->enabled = false;
                    return cur;
                }
            }
            return nullptr;
        };
        if (U *s = scan(size()))
            return s;
        for (;;) {
            auto index = reserve();
            if (!index)
                break;
            if (slot(*index).load())
                continue; // Occupied past the scan (manual plant); skip it.
            U *fresh = new U();
            fresh->allocated = true;
            fresh->enabled = false;
            fresh->id = (int)*index;
            U *expect = nullptr;
            // CAS, not store: a racing scan may claim this slot first.
            if (slot(*index).compare_exchange_strong(expect, fresh)) {
                extend_to(*index + 1);
                return fresh;
            }
            delete fresh;
        }
        // Full: one more scan in case a slot freed mid-claim.
        return scan(size());
    }

    // Load the value at index; writes must use slot() or publish().
    T operator[](size_t index) const { return load(index); }

    // Raw slot for CAS-claim loops; publishes its bucket on first touch.
    std::atomic<T> &slot(size_t index) {
        Bucket *b = bucket_for(index >> kBucketBits);
        return b->slots[index & (kBucketSize - 1)];
    }

    // Live atomic slots for CAS-claim loops (see obtain()).
    auto slots() {
        return std::views::iota(size_t{0}, std::min(size(), capacity())) |
               std::views::transform([this](size_t index) -> std::atomic<T> & {
                   return slot(index);
               });
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
        T operator*() const { return pool_->load(index_); }
        iterator &operator++() {
            ++index_;
            return *this;
        }
        iterator operator++(int) {
            iterator c = *this;
            ++index_;
            return c;
        }
        bool operator==(const iterator &o) const { return index_ == o.index_; }

      private:
        friend class SegmentedPool;
        iterator(const SegmentedPool *p, size_t index)
            : pool_(p), index_(index) {}
        const SegmentedPool *pool_;
        size_t index_;
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

    T load(size_t index) const {
        Bucket *b = bucket_for(index >> kBucketBits);
        return b->slots[index & (kBucketSize - 1)].load(
            std::memory_order_acquire);
    }

    // Mutable: first touch publishes buckets even through const loads.
    mutable std::array<std::atomic<Bucket *>, MaxSegs> buckets_;
    std::atomic<size_t> size_{0};
};

#endif
