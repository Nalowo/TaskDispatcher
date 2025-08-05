#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include "queue/bounded_queue.hpp"

using namespace dispatcher::queue;

TEST(BoundedQueueTest, InitiallyEmpty) {
    BoundedQueue q(2);
    EXPECT_FALSE(q.try_pop().has_value());
}

TEST(BoundedQueueTest, SinglePushPop) {
    BoundedQueue q(2);
    bool called = false;
    q.push([&]{ called = true; });
    auto t = q.try_pop();
    ASSERT_TRUE(t.has_value());
    (*t)();
    EXPECT_TRUE(called);
}

TEST(BoundedQueueTest, OrderPreserved) {
    BoundedQueue q(3);
    std::vector<int> seq;
    q.push([&]{ seq.push_back(10); });
    q.push([&]{ seq.push_back(20); });
    q.push([&]{ seq.push_back(30); });
    for (int i = 0; i < 3; ++i) {
        auto t = q.try_pop();
        ASSERT_TRUE(t.has_value());
        (*t)();
    }
    EXPECT_EQ((std::vector<int>{10,20,30}), seq);
}

TEST(BoundedQueueTest, BlocksWhenFull) {
    BoundedQueue q(1);
    std::atomic<bool> pushed{false};
    // первый займёт слот
    q.push([]{});
    std::thread t([&]{
        q.push([&]{ pushed = true; });
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    EXPECT_FALSE(pushed.load());
    // освободить слот
    auto t1 = q.try_pop(); 
    ASSERT_TRUE(t1.has_value());
    EXPECT_FALSE(pushed.load());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    t1 = q.try_pop();
    ASSERT_TRUE(t1.has_value());
    (*t1)();
    EXPECT_TRUE(pushed.load());
    t.join();
}

TEST(BoundedQueueTest, ConcurrentPop) {
    const int N = 100;
    BoundedQueue q(N);
    for (int i = 0; i < N; ++i) q.push([]{});
    std::atomic<int> cnt{0};
    auto consumer = [&]{ while (auto t = q.try_pop()) ++cnt; };
    std::thread c1(consumer), c2(consumer);
    c1.join(); c2.join();
    EXPECT_EQ(N, cnt.load());
}
