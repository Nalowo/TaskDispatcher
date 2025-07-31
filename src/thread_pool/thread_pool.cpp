#include "thread_pool/thread_pool.hpp"
#include "logger.hpp"

namespace dispatcher::thread_pool {

    ThreadPool::ThreadPool(std::shared_ptr<dispatcher::queue::PriorityQueue> pq, size_t threads)
        : pq_(std::move(pq)), threadsCount(threads) 
    {
        Run();
    }

    void ThreadPool::Run()
    {
        for (size_t i = 0; i < threadsCount; ++i) {
            workers_.emplace_back([this] {
                while (auto task = pq_->pop()) {
                    try {
                        (*task)();
                    } catch (const std::exception& e) {
                        Logger::Get().Log(e.what());
                    } catch (...) {
                        Logger::Get().Log("Unknown exception in task");
                    }
                }
            });
        }
    }

    ThreadPool::~ThreadPool() {
        pq_->shutdown();
        for (auto& t : workers_) {
            if (t.joinable())
                t.join();
        }
    }

} // namespace dispatcher::thread_pool