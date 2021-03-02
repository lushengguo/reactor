#include "base/ThreadPool.hpp"
#include "base/log.hpp"

using namespace reactor;
void ThreadPool::run_in_thread()
{
    while (true)
    {
        compete_cond_.wait();
        log_trace("thread id=%ld wake up", pthread_self());
        task_queue_mutex_.lock();
        log_trace("thread id=%ld lock the queue mutex", pthread_self());
        Task task;
        if (!tasks_.empty())
        {
            task = std::move(tasks_.front());
            tasks_.pop_front();
            log_trace("thread id=%ld get the task", pthread_self());
        }
        task_queue_mutex_.unlock();
        log_trace("thread id=%ld unlock the queue mutex", pthread_self());
        if (task)
        {
            task();
        }
        log_trace("thread id=%ld call task finish", pthread_self());
    }
}
bool ThreadPool::add_task(Task &&task)
{
    MutexLockGuard lock(&task_queue_mutex_);
    if (tasks_.size() >= task_size_)
    {
        log_debug("pThreadPool task queue is full");
        return false;
    }
    else
    {
        log_debug("add task to pThreadPool task queue ,call later");
        tasks_.push_back(std::move(task));
    }

    if (!tasks_.empty())
    {
        compete_cond_.broadcast();
    }

    return true;
}

bool ThreadPool::add_task(Task &task)
{
    MutexLockGuard lock(&task_queue_mutex_);
    if (tasks_.size() >= task_size_)
    {
        log_debug("pThreadPool task queue is full");
        return false;
    }
    else
    {
        log_debug("add task to pThreadPool task queue ,call later");
        tasks_.push_back(std::move(task));
    }
    return true;
}

void ThreadPool::start()
{
    for (size_t i = 0; i < thread_num_; i++)
    {
        threads_.emplace_back(
          new Thread(std::bind(&ThreadPool::run_in_thread, this)));
        threads_[i]->start();
    }
    running_ = true;
}