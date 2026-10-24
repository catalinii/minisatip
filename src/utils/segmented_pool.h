#ifndef SEGMENTED_POOL_H
#define SEGMENTED_POOL_H

#include <algorithm>
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
    { u->enabled } -> std::convertible_to<bool>;
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
            b = nullptr;
    }
    // Mid-life teardown: length to 0 first, then delete and
    // blank every published slot. Single-threaded use only.
    void destroy_all() {
        size_ = 0;
        for_each_published([](std::atomic<T> &s) {
            if constexpr (std::is_pointer_v<T>) {
                delete static_cast<T>(s);
                s = nullptr;
            } else {
                s = T{};
            }
        });
    }
    // Destruction runs teardown, then releases the buckets.
    // Manual deletes must blank; no stack survivors.
    ~SegmentedPool() {
        destroy_all();
        for (auto &b : buckets_)
            delete static_cast<Bucket *>(b);
    }
    SegmentedPool(const SegmentedPool &) = delete;
    SegmentedPool &operator=(const SegmentedPool &) = delete;

    size_t size() const { return size_; }
    bool empty() const { return size() == 0; }

    // Reserve the next index; nullopt when full. Publish with slot();
    // the slot reads default until then.
    std::optional<size_t> reserve() {
        size_t index = size_.fetch_add(1);
        if (index >= capacity()) {
            // Paired with our own increment; size stays above live slots.
            size_.fetch_sub(1);
            return std::nullopt;
        }
        return index;
    }

    // Lower the length to n; slots stay allocated for later regrowth.
    void shrink_to(size_t n) {
        size_t cur = size_;
        while (cur > n && !size_.compare_exchange_weak(cur, n))
            ;
    }

    // Raise the length to n without writing slots; false over capacity.
    bool extend_to(size_t n) {
        if (n > capacity())
            return false;
        size_t cur = size_;
        while (cur < n && !size_.compare_exchange_weak(cur, n))
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
                U *cur = slot(index);
                if (!cur) {
                    U *fresh = new U();
                    fresh->allocated = true;
                    fresh->enabled = false;
                    fresh->id = static_cast<int>(index);
                    if (slot(index).compare_exchange_strong(cur, fresh))
                        return fresh;
                    delete fresh;
                    cur = slot(index);
                    if (!cur)
                        continue;
                }
                bool expected = false;
                if (!cur->enabled &&
                    cur->allocated.compare_exchange_strong(expected, true)) {
                    // id is set once at allocation; rewriting it here races
                    // with unlocked x->id readers (TSAN).
                    cur->enabled = false;
                    return cur;
                }
            }
            return nullptr;
        };
        // Clamp: a failing reserve transiently overshoots capacity.
        if (U *s = scan(std::min(size(), capacity())))
            return s;
        for (;;) {
            auto index = reserve();
            if (!index)
                break;
            if (slot(*index))
                continue; // Occupied past the scan (manual plant); skip it.
            U *fresh = new U();
            fresh->allocated = true;
            fresh->enabled = false;
            fresh->id = static_cast<int>(*index);
            U *expect = nullptr;
            // CAS, not store: a racing scan may claim this slot first.
            if (slot(*index).compare_exchange_strong(expect, fresh)) {
                extend_to(*index + 1);
                return fresh;
            }
            delete fresh;
        }
        // Full: one more scan in case a slot freed mid-claim.
        return scan(std::min(size(), capacity()));
    }

    // Load the value at index; write via slot(), covering with extend_to().
    T operator[](size_t index) const { return load(index); }

    // Raw slot for CAS-claim loops; publishes its bucket on first touch.
    std::atomic<T> &slot(size_t index) {
        Bucket *b = bucket_for(index >> kBucketBits);
        return b->slots[index & (kBucketSize - 1)];
    }

    // Visit every slot in published buckets, past shrinks; for teardown.
    template <typename F> void for_each_published(F &&f) {
        for (size_t b = 0; b < MaxSegs; ++b) {
            Bucket *bk = buckets_[b];
            if (!bk)
                continue;
            for (auto &s : bk->slots)
                f(s);
        }
    }

    // Single-threaded reset: blank all published slots, drop the length.
    void clear() {
        for_each_published([](std::atomic<T> &s) { s = T{}; });
        size_ = 0;
    }

    class iterator {
      public:
        using value_type = T;
        using difference_type = ptrdiff_t;
        using iterator_category = std::forward_iterator_tag;
        iterator() = default;
        // Snapshot range over live objects; each deref reloads.
        // Slots past the snapshot end yield raw, as before.
        T operator*() const { return pool_->load(index_); }
        iterator &operator++() {
            ++index_;
            skip_to_live();
            return *this;
        }
        iterator operator++(int) {
            iterator c = *this;
            ++(*this);
            return c;
        }
        bool operator==(const iterator &o) const { return index_ == o.index_; }

      private:
        friend class SegmentedPool;
        iterator(const SegmentedPool *p, size_t index, size_t end)
            : pool_(p), index_(index), end_(end) {
            skip_to_live();
        }
        // Advance past slots the default iteration hides.
        void skip_to_live() {
            while (index_ < end_ && !SegmentedPool::live(pool_->load(index_)))
                ++index_;
        }
        const SegmentedPool *pool_ = nullptr;
        size_t index_ = 0, end_ = 0;
    };

    iterator begin() const {
        size_t end = std::min(size(), capacity());
        return {this, 0, end};
    }
    iterator end() const {
        // Clamp: a failed reserve can transiently overshoot capacity.
        size_t end = std::min(size(), capacity());
        return {this, end, end};
    }

  private:
    struct Bucket {
        std::array<std::atomic<T>, kBucketSize> slots;
        // Atomics don't self-init; blank before publication.
        Bucket() {
            for (auto &s : slots)
                s = T{};
        }
    };

    Bucket *bucket_for(size_t b) const {
        Bucket *found = buckets_[b];
        if (found)
            return found;
        Bucket *fresh = new Bucket();
        Bucket *want = nullptr;
        if (buckets_[b].compare_exchange_strong(want, fresh))
            return fresh;
        delete fresh;
        return want;
    }

    T load(size_t index) const {
        Bucket *b = bucket_for(index >> kBucketBits);
        return b->slots[index & (kBucketSize - 1)];
    }

    // Live for default iteration: enabled objects, or every value
    // when T has no flag (int test pools, raw int* slots).
    static bool live(T v) {
        if constexpr (std::is_pointer_v<T> &&
                      ReusableObject<std::remove_pointer_t<T>>)
            return v && v->enabled;
        else
            return true;
    }

    // Mutable: first touch publishes buckets even through const loads.
    mutable std::array<std::atomic<Bucket *>, MaxSegs> buckets_;
    std::atomic<size_t> size_{0};
};

#endif
