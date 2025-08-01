#include "task_dispatcher.hpp"

namespace dispatcher {

TaskDispatcher::TaskDispatcher(size_t thread_count, OptionsType opts)
{
    pq_ = std::make_shared<dispatcher::queue::PriorityQueue>(opts);
    pool_ = std::make_unique<thread_pool::ThreadPool>(pq_, thread_count);
}

void TaskDispatcher::schedule(TaskPriority prio, std::function<void()> task) 
{
    pq_->push(prio, std::move(task));
}

void TaskDispatcher::shutdown()
{
    pq_->shutdown();
}

TaskDispatcher::~TaskDispatcher() = default;

} // namespace dispatcher