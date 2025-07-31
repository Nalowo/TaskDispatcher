#include "queue/priority_queue.hpp"

namespace dispatcher::queue {

    PriorityQueue::PriorityQueue(const std::unordered_map<TaskPriority, QueueOptions>& opts) 
    {
        for (auto [prio, opt] : opts) {
            if (opt.bounded && opt.capacity) {
                queues_[static_cast<int>(prio)] = std::make_unique<BoundedQueue>(*opt.capacity);
            } else {
                queues_[static_cast<int>(prio)] = std::make_unique<UnboundedQueue>();
            }
        }
    }

    void PriorityQueue::push(TaskPriority priority, std::function<void()> task)
    {
        if (static_cast<int>(priority) >= static_cast<int>(TaskPriority::Count))
            throw std::invalid_argument("Wrong priority index");
        {
            std::lock_guard<std::mutex> lk(mtx_);
            queues_[static_cast<int>(priority)]->push(std::move(task));
            ++taskCount_;
        }
        cv_.notify_one();
    }

    std::optional<std::function<void()>> PriorityQueue::pop()
    {
        std::optional<std::function<void()>> task;
        {
            std::unique_lock<std::mutex> lk(mtx_);
            cv_.wait(lk, [this] { return shutdown_ || taskCount_ > 0; });
            
            if (shutdown_ && taskCount_ == 0)
                return task;
            
            for (auto& q : queues_)
            {
                if (!q)
                    continue;

                if (task = q->try_pop(); task)
                {
                    --taskCount_;
                    break;
                }
            }
        }
        return task;
    }

    void PriorityQueue::shutdown()
    {
        {
            std::lock_guard<std::mutex> lk(mtx_);
            shutdown_ = true;
        }
        cv_.notify_all();
    }
} // namespace dispatcher::queue