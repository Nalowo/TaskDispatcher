#pragma once

#include "queue/priority_queue.hpp"
#include <atomic>
#include <functional>
#include <memory>
#include <thread>
#include <vector>

namespace dispatcher::thread_pool {

class ThreadPool {
public:
    explicit ThreadPool(std::shared_ptr<dispatcher::queue::PriorityQueue> pq, size_t threads);
    ~ThreadPool();

private:
    void Run();

    std::shared_ptr<dispatcher::queue::PriorityQueue> pq_;
    std::vector<std::jthread> workers_;
    size_t threadsCount = 0;
};

} // namespace dispatcher::thread_pool
