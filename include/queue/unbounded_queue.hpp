#pragma once
#include "queue/queue.hpp"

#include <mutex>
#include <queue>

namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
public:
    using task = std::function<void()>;
    using container = std::queue<task>;

    explicit UnboundedQueue();

    void push(task task) override;

    std::optional<task> try_pop() override;

private:
    container queue_;
    std::mutex mtx_;
};

}  // namespace dispatcher::queue