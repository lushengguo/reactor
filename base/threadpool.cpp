#include "base/threadpool.hpp"
#include "base/log.hpp"
#include <atomic>

using namespace reactor;
void ThreadPool::run_in_thread()
{
    while (true)
    {
        compete_cond_.wait();
        task_queue_mutex_.lock();
        Task task;
        if (!tasks_.empty())
        {
            task = std::move(tasks_.front());
            tasks_.pop_front();
        }
        task_queue_mutex_.unlock();
        if (task)
        {
            task();
        }
    }
}
bool ThreadPool::add_task(Task &&task)
{
    MutexLockGuard lock(task_queue_mutex_);
    if (tasks_.size() >= task_size_)
    {
        log_debug("pThreadPool task queue is full");
        return false;
    }
    else
    {
        // log_debug("add task to work thread");
        tasks_.emplace_back(std::move(task));
    }

    if (!tasks_.empty())
    {
        compete_cond_.broadcast();
    }

    return true;
}

bool ThreadPool::add_task(const Task &task)
{
    MutexLockGuard lock(task_queue_mutex_);
    if (tasks_.size() >= task_size_)
    {
        log_debug("pThreadPool task queue is full");
        return false;
    }
    else
    {
        // log_debug("add task to work thread");
        tasks_.emplace_back(task);
    }

    if (!tasks_.empty())
    {
        compete_cond_.broadcast();
    }

    return true;
}

void ThreadPool::start()
{
    if (running_.load(std::memory_order_acquire))
        return;

    threads_.reserve(thread_num_);
    running_.store(true, std::memory_order_release);
    for (size_t i = 0; i < thread_num_; i++)
    {
        threads_.emplace_back(std::bind(&ThreadPool::run_in_thread, this));
    }
    compete_cond_.broadcast();
}

void ThreadPool::end()
{
    running_.store(true, std::memory_order_release);
}