#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include "queue/unbounded_queue.hpp"

using namespace dispatcher::queue;

TEST(UnboundedQueueTest, InitiallyEmpty) {
    UnboundedQueue q;
    EXPECT_FALSE(q.try_pop().has_value());
}

TEST(UnboundedQueueTest, SinglePushPop) {
    UnboundedQueue q;
    bool called = false;
    q.push([&]{ called = true; });
    auto t = q.try_pop();
    ASSERT_TRUE(t.has_value());
    (*t)();
    EXPECT_TRUE(called);
    EXPECT_FALSE(q.try_pop().has_value());
}

TEST(UnboundedQueueTest, OrderPreserved) {
    UnboundedQueue q;
    std::vector<int> seq;
    q.push([&]{ seq.push_back(1); });
    q.push([&]{ seq.push_back(2); });
    q.push([&]{ seq.push_back(3); });
    for (int i = 0; i < 3; ++i) {
        auto t = q.try_pop();
        ASSERT_TRUE(t.has_value());
        (*t)();
    }
    EXPECT_EQ((std::vector<int>{1,2,3}), seq);
}

TEST(UnboundedQueueTest, TryPopNonBlocking) {
    UnboundedQueue q;
    auto start = std::chrono::steady_clock::now();
    auto t = q.try_pop();
    auto dur = std::chrono::steady_clock::now() - start;
    EXPECT_FALSE(t.has_value());
    EXPECT_LT(dur, std::chrono::milliseconds(5));
}

TEST(UnboundedQueueTest, ConcurrentPushPop) {
    UnboundedQueue q;
    const int N = 500;
    std::atomic<int> counter{0};
    std::thread prod([&]{
        for (int i = 0; i < N; ++i)
            q.push([&]{ counter.fetch_add(1, std::memory_order_relaxed); });
    });
    int popped = 0;
    while (popped < N) {
        auto t = q.try_pop();
        if (t) {
            (*t)();
            ++popped;
        }
    }
    prod.join();
    EXPECT_EQ(N, counter.load());
}
