#include "queue/unbounded_queue.hpp"

namespace dispatcher::queue {

UnboundedQueue::UnboundedQueue(size_t capacity) {}

void UnboundedQueue::push(std::function<void()> task) {
    std::lock_guard<std::mutex> lk(mtx_);
    queue_.push(std::move(task));
    cv_.notify_one();
}

std::optional<UnboundedQueue::task> UnboundedQueue::try_pop() {
    std:std::unique_lock<std::mutex> lk(mtx_);
    cv_.wait(lk, [this] { return !queue_.empty(); });

    if (queue_.empty())
        return std::nullopt;

    auto task = std::move(queue_.front());
    queue_.pop();
    return task;
}
}  // namespace dispatcher::queue