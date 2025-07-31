#include "queue/unbounded_queue.hpp"

namespace dispatcher::queue {

UnboundedQueue::UnboundedQueue() {}

void UnboundedQueue::push(std::function<void()> task) {
    std::lock_guard<std::mutex> lk(mtx_);
    queue_.push(std::move(task));
}

std::optional<UnboundedQueue::task> UnboundedQueue::try_pop() {
    std::optional<UnboundedQueue::task> task;
    {
        std::lock_guard<std::mutex> lk(mtx_);

        if (queue_.empty())
            return task;
        
        task = std::move(queue_.front());
        queue_.pop();
    }
    return task;
}
}  // namespace dispatcher::queue