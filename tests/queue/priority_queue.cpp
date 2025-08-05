#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include "queue/priority_queue.hpp"

using namespace dispatcher::queue;
using dispatcher::TaskPriority;

static auto makeOpts() {
    return std::unordered_map<TaskPriority, QueueOptions>{
        {TaskPriority::High,   {false, std::nullopt}},
        {TaskPriority::Normal, {false, std::nullopt}}
    };
}

TEST(PriorityQueueTest, PopBlocksUntilPush) {
    PriorityQueue pq(makeOpts());
    std::atomic<bool> done{false};
    std::thread t([&]{
        auto task = pq.pop();
        ASSERT_TRUE(task.has_value());
        (*task)();
        done = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    EXPECT_FALSE(done);
    pq.push(TaskPriority::High, [&]{ done = true; });
    t.join();
    EXPECT_TRUE(done);
}

TEST(PriorityQueueTest, ShutdownUnblocksPop) {
    PriorityQueue pq(makeOpts());
    std::atomic<bool> unblocked{false};
    std::thread t([&]{
        auto task = pq.pop();
        EXPECT_FALSE(task.has_value());
        unblocked = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    pq.shutdown();
    t.join();
    EXPECT_TRUE(unblocked);
}

TEST(PriorityQueueTest, HighPriorityFirst) {
    PriorityQueue pq(makeOpts());
    std::vector<int> order;
    pq.push(TaskPriority::Normal, [&]{ order.push_back(1); });
    pq.push(TaskPriority::High,   [&]{ order.push_back(2); });
    for (int i = 0; i < 2; ++i) {
        auto t = pq.pop();
        ASSERT_TRUE(t.has_value());
        (*t)();
    }
    EXPECT_EQ((std::vector<int>{2,1}), order);
}

TEST(PriorityQueueTest, MixedInterleaving) {
    PriorityQueue pq(makeOpts());
    std::vector<int> seq;
    pq.push(TaskPriority::Normal, [&]{ seq.push_back(1); });
    pq.push(TaskPriority::Normal, [&]{ seq.push_back(2); });
    pq.push(TaskPriority::High,   [&]{ seq.push_back(3); });
    pq.push(TaskPriority::Normal, [&]{ seq.push_back(4); });
    pq.push(TaskPriority::High,   [&]{ seq.push_back(5); });
    for (int i = 0; i < 5; ++i) {
        auto t = pq.pop();
        ASSERT_TRUE(t.has_value());
        (*t)();
    }
    EXPECT_EQ((std::vector<int>{3,5,1,2,4}), seq);
}

TEST(PriorityQueueTest, InvalidPriorityThrows) {
    PriorityQueue pq(makeOpts());
    EXPECT_THROW(pq.push(static_cast<TaskPriority>(-1), []{}), std::invalid_argument);
}
