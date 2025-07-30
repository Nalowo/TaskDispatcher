#include "queue/bounded_queue.hpp"

namespace dispatcher::queue {

BoundedQueue::BoundedQueue(size_t capacity) : capacity_(capacity) {}

void BoundedQueue::push(std::function<void()> task) {
    std::unique_lock<std::mutex> lk(mtx_);
    cv_full_.wait(lk, [this] { return queue_.size() < capacity_; });
    queue_.push(std::move(task));
    cv_empty_.notify_one();
}

std::optional<std::function<void()>> BoundedQueue::try_pop() {
    std::unique_lock<std::mutex> lk(mtx_);
    cv_empty_.wait(lk, [this]{ return !queue_.empty(); });
    
    if (queue_.empty())
        return std::nullopt;
    
    auto task = std::move(queue_.front());
    queue_.pop();
    cv_full_.notify_one();
    return task;
}
}  // namespace dispatcher::queue