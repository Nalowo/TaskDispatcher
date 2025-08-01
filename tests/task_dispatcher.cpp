#include <gtest/gtest.h>
#include <atomic>
#include <vector>
#include "task_dispatcher.hpp"
#include "types.hpp"

using namespace dispatcher;

TEST(TaskDispatcherTest, ExecutesNormalTasks) {
    TaskDispatcher td(2);
    std::atomic<int> cnt{0};
    td.schedule(TaskPriority::Normal, [&]{ cnt.fetch_add(1); });
    td.schedule(TaskPriority::Normal, [&]{ cnt.fetch_add(1); });
    td.shutdown();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_EQ(2, cnt.load());
}

TEST(TaskDispatcherTest, HighBeforeNormal) {
    TaskDispatcher td(1);
    std::vector<int> seq;
    td.schedule(TaskPriority::Normal, [&]{ seq.push_back(1); });
    td.schedule(TaskPriority::High,   [&]{ seq.push_back(2); });
    td.schedule(TaskPriority::Normal, [&]{ seq.push_back(3); });
    td.shutdown();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_EQ((std::vector<int>{2,1,3}), seq);
}

TEST(TaskDispatcherTest, MultiThreadThroughput) {
    const int N = 100;
    TaskDispatcher td(4);
    std::atomic<int> cnt{0};
    for (int i = 0; i < N; ++i)
        td.schedule(TaskPriority::Normal, [&]{ cnt.fetch_add(1); });
    td.shutdown();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_EQ(N, cnt.load());
}

TEST(TaskDispatcherTest, ExceptionSafety) {
    TaskDispatcher td(2);
    std::atomic<bool> ran{false};
    td.schedule(TaskPriority::Normal, []{ throw std::runtime_error("fail"); });
    td.schedule(TaskPriority::Normal, [&]{ ran = true; });
    td.shutdown();
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    EXPECT_TRUE(ran.load());
}

TEST(TaskDispatcherTest, DefaultOptionsDoesNotCrash) {
    TaskDispatcher td(std::thread::hardware_concurrency());
    td.shutdown();
    SUCCEED();
}
