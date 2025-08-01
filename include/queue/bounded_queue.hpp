#pragma once
#include "queue/queue.hpp"

#include <condition_variable>
#include <mutex>
#include <vector>
#include <queue>

namespace dispatcher::queue {

class BoundedQueue : public IQueue {
public:
    using task = std::function<void()>;
    using container = std::queue<task>;

    explicit BoundedQueue(size_t capacity);
    void push(task task) override;
    std::optional<task> try_pop() override;

private:
    container queue_;
    std::mutex mtx_;
    std::condition_variable cv_full_;
    size_t capacity_;
};

}  // namespace dispatcher::queue