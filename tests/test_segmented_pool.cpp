/*
 * Copyright (C) 2014-2024 Catalin Toda <catalinii@yahoo.com> et al
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 */
#include "opts.h"
#include "utils.h"
#include "utils/segmented_pool.h"
#include "utils/testing.h"
#include <algorithm>
#include <atomic>
#include <chrono>
#include <string.h>
#include <thread>
#include <vector>

// Value append on reserve()+slot(); production objects use obtain().
template <size_t M>
std::optional<size_t> append(SegmentedPool<int, M> &pool, int v) {
    auto idx = pool.reserve();
    if (idx)
        pool.slot(*idx) = v;
    return idx;
}

int test_append_and_iterate() {
    SegmentedPool<int, 16> pool; // 600 appends cross 10 buckets of 64
    for (int i = 0; i < 600; ++i) {
        auto idx = append(pool, i);
        ASSERT(idx.has_value(), "append failed before capacity");
        ASSERT_EQUAL(*idx, static_cast<size_t>(i), "append index mismatch");
    }
    ASSERT_EQUAL(pool.size(), 600, "size mismatch after appends");

    int expect = 0;
    for (int v : pool) {
        ASSERT_EQUAL(v, expect, "iteration order mismatch");
        ++expect;
    }
    ASSERT_EQUAL(expect, 600, "iteration count mismatch");
    return 0;
}

int test_extend_and_defaults() {
    SegmentedPool<int *, 2> pool;
    ASSERT(pool.extend_to(10), "extend_to failed");
    ASSERT_EQUAL(pool.size(), 10, "size mismatch after extend");
    for (int *v : pool)
        ASSERT(v == nullptr, "extended slot is not default");
    ASSERT(!pool.extend_to(100000), "extend_to over capacity succeeded");
    return 0;
}

int test_snapshot_iterator() {
    SegmentedPool<int, 8> pool;
    for (int i = 0; i < 300; ++i)
        append(pool, i);
    auto first = pool.begin(), last = pool.end();
    for (int i = 300; i < 400; ++i)
        append(pool, i);
    size_t count = 0;
    for (auto it = first; it != last; ++it)
        ++count;
    ASSERT_EQUAL(count, 300, "iterator did not hold its snapshot");
    ASSERT_EQUAL(pool.size(), 400, "size mismatch after late appends");
    return 0;
}

int test_overflow() {
    SegmentedPool<int, 1> pool; // capacity is exactly 64
    for (int i = 0; i < 64; ++i)
        ASSERT(append(pool, i).has_value(), "append failed when filling");
    ASSERT(!append(pool, 999).has_value(), "append over capacity worked");
    ASSERT_EQUAL(pool.size(), 64, "size moved past capacity");
    size_t count = 0;
    for (int v : pool) {
        (void)v;
        ++count;
    }
    ASSERT_EQUAL(count, 64, "iteration count mismatch at capacity");
    return 0;
}

int test_clear() {
    SegmentedPool<int, 8> pool;
    for (int i = 1; i <= 300; ++i)
        append(pool, i);
    pool.clear();
    ASSERT_EQUAL(pool.size(), 0, "size is not zero after clear");
    ASSERT(append(pool, 7).has_value(), "append failed after clear");
    ASSERT_EQUAL(*pool.begin(), 7, "wrong value after clear");
    return 0;
}

int test_shrink_and_published() {
    SegmentedPool<int, 8> pool;
    for (int i = 1; i <= 200; ++i)
        append(pool, i);
    pool.shrink_to(10);
    ASSERT_EQUAL(pool.size(), 10, "size mismatch after shrink");
    size_t visits = 0, nonzero = 0;
    pool.for_each_published([&](std::atomic<int> &s) {
        ++visits;
        nonzero += s != 0;
    });
    // Four published buckets are visited whole, past the shrink.
    ASSERT_EQUAL(visits, 256, "for_each_published missed slots");
    ASSERT_EQUAL(nonzero, 200, "for_each_published missed values");
    pool.clear();
    ASSERT_EQUAL(pool.size(), 0, "size is not zero after clear");
    nonzero = 0;
    pool.for_each_published([&](std::atomic<int> &s) { nonzero += s != 0; });
    ASSERT_EQUAL(nonzero, 0, "clear left values past the shrink");
    ASSERT(append(pool, 9).has_value(), "append failed after clear");
    ASSERT_EQUAL(pool[0], 9, "wrong value after clear");
    return 0;
}

int test_concurrent_claim() {
    SegmentedPool<int, 1> pool; // 64 slots claimed via CAS, obtain-style
    ASSERT(pool.extend_to(64), "extend_to failed");
    std::atomic<int> claimed{0};
    auto claim = [&](int id) {
        size_t n = pool.size();
        for (size_t i = 0; i < n; ++i) {
            int want = 0;
            if (pool.slot(i).compare_exchange_strong(want, id))
                claimed.fetch_add(1, std::memory_order_relaxed);
        }
    };
    std::thread t1(claim, 1), t2(claim, 2), t3(claim, 3), t4(claim, 4);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    ASSERT_EQUAL(claimed, 64, "slot claimed twice or never");
    for (int v : pool)
        ASSERT(v >= 1 && v <= 4, "slot left unclaimed");
    return 0;
}

int test_concurrent_append_iterate() {
    SegmentedPool<int, 160> pool;
    std::atomic<bool> done{false};
    auto add = [&](int id) {
        for (int i = 0; i < 2000; ++i)
            append(pool, id * 100000 + i);
    };
    auto read = [&]() {
        while (!done) // stress publication; only counts, never fails
            for (int v : pool) {
                (void)v;
            }
    };
    std::thread p1(add, 1), p2(add, 2), p3(add, 3), p4(add, 4);
    std::thread r1(read), r2(read);
    p1.join();
    p2.join();
    p3.join();
    p4.join();
    done = true;
    r1.join();
    r2.join();
    ASSERT_EQUAL(pool.size(), 8000, "size mismatch after concurrent appends");

    std::vector<int> got(pool.begin(), pool.end()), want;
    for (int id = 1; id <= 4; ++id)
        for (int i = 0; i < 2000; ++i)
            want.push_back(id * 100000 + i);
    std::sort(got.begin(), got.end());
    std::sort(want.begin(), want.end());
    ASSERT(got == want, "appended values lost or duplicated");
    return 0;
}

struct Counted {
    static inline int alive = 0;
    Counted() { ++alive; }
    ~Counted() { --alive; }
};

int test_destroy_all() {
    SegmentedPool<Counted *, 2> pool;
    ASSERT(pool.extend_to(5), "extend_to failed");
    for (size_t i = 0; i < 5; ++i)
        pool.slot(i) = new Counted();
    ASSERT_EQUAL(Counted::alive, 5, "setup mismatch");
    pool.destroy_all();
    ASSERT_EQUAL(Counted::alive, 0, "destroy_all leaked elements");
    ASSERT_EQUAL(pool.size(), 0, "destroy_all left length behind");
    bool blank = true;
    pool.for_each_published([&](auto &s) { blank = blank && !s; });
    ASSERT(blank, "destroy_all left a slot behind");
    {
        SegmentedPool<Counted *, 2> dying;
        ASSERT(dying.extend_to(3), "extend_to failed");
        for (size_t i = 0; i < 3; ++i)
            dying.slot(i) = new Counted();
    }
    ASSERT_EQUAL(Counted::alive, 0, "pool destructor leaked elements");
    return 0;
}

struct Item {
    int id = -1;
    std::atomic<bool> enabled{false};
    std::atomic<bool> allocated{false};
};

int test_enabled_iteration() {
    SegmentedPool<Item *, 1> pool;
    ASSERT(pool.extend_to(4), "extend_to failed");
    for (size_t i = 0; i < 4; ++i) {
        pool.slot(i) = new Item();
        pool[i]->id = static_cast<int>(i);
    }
    pool[1]->enabled = true;
    pool[3]->enabled = true;
    std::vector<int> got;
    for (Item *it : pool)
        got.push_back(it->id);
    ASSERT(got == std::vector<int>({1, 3}), "default must skip disabled");
    return 0;
}

int test_obtain_full_contention() {
    SegmentedPool<Item *, 1> pool; // 64 slots, driven full
    for (int i = 0; i < 64; ++i) {
        Item *it = pool.obtain();
        ASSERT(it, "setup obtain failed");
        it->enabled = true;
    }
    std::atomic<bool> stop{false};
    std::atomic<int> fails{0};
    auto hammer = [&]() {
        while (!stop) {
            if (!pool.obtain())
                fails.fetch_add(1, std::memory_order_relaxed);
        }
    };
    std::thread t1(hammer), t2(hammer), t3(hammer), t4(hammer);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    stop = true;
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    ASSERT(fails > 0, "hammers never ran");
    return 0;
}

int main() {
    opts.log = 255;
    strcpy(thread_info[thread_index].thread_name, "test_segmented_pool");
    TEST_FUNC(test_append_and_iterate(), "append across buckets and iterate");
    TEST_FUNC(test_extend_and_defaults(), "extend with default slots");
    TEST_FUNC(test_snapshot_iterator(), "iterator holds length snapshot");
    TEST_FUNC(test_overflow(), "append past capacity fails cleanly");
    TEST_FUNC(test_clear(), "clear resets the pool");
    TEST_FUNC(test_destroy_all(), "destroy_all deletes and blanks");
    TEST_FUNC(test_enabled_iteration(), "default iteration skips disabled");
    TEST_FUNC(test_obtain_full_contention(),
              "contended obtains on a full pool");
    TEST_FUNC(test_shrink_and_published(), "shrink keeps teardown covered");
    TEST_FUNC(test_concurrent_claim(), "concurrent CAS claims");
    TEST_FUNC(test_concurrent_append_iterate(),
              "concurrent append and iterate");
    return 0;
}
