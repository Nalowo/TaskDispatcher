#pragma once

#include <memory>
#include <unordered_map>

#include "queue/priority_queue.hpp"
#include "thread_pool/thread_pool.hpp"
#include "types.hpp"

namespace dispatcher {

class TaskDispatcher {
    using OptionsType = std::unordered_map<TaskPriority, dispatcher::queue::QueueOptions>;
public:
    explicit TaskDispatcher(size_t thread_count,
        OptionsType opts = {
            {TaskPriority::High, {true, 1000}},
            {TaskPriority::Normal, {false, std::nullopt}}
        });

    void schedule(TaskPriority prio, std::function<void()> task);
    void shutdown();

    ~TaskDispatcher();

private:
    std::shared_ptr<dispatcher::queue::PriorityQueue> pq_;
    std::unique_ptr<thread_pool::ThreadPool> pool_;
};

}  // namespace dispatcher