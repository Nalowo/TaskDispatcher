#pragma once
#include "queue/bounded_queue.hpp"
#include "queue/unbounded_queue.hpp"
#include "types.hpp"

#include <atomic>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace dispatcher::queue {

class PriorityQueue {
public:
    explicit PriorityQueue(const std::unordered_map<TaskPriority, QueueOptions>& opts);

    void push(TaskPriority priority, std::function<void()> task);
    // block on pop until shutdown is called
    // after that return std::nullopt on empty queue
    std::optional<std::function<void()>> pop();

    void shutdown();

private:
    std::array<std::unique_ptr<IQueue>, static_cast<int>(TaskPriority::Count)> queues_;
    std::mutex mtx_;
    std::condition_variable cv_;
    size_t taskCount_ = 0;
    bool shutdown_ = false;
};

}  // namespace dispatcher::queue