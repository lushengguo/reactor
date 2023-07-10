#pragma once
#ifndef REACTOR_THREADPOLL_HPP
#define REACTOR_THREADPOLL_HPP
#include "base/condition.hpp"
#include "base/thread.hpp"
#include <atomic>
#include <functional>
#include <queue>
#include <thread>
namespace reactor
{
class ThreadPool
{
  public:
    typedef std::function<void()> Task;
    ThreadPool(size_t max_task = 20, int thread_num = 10) : task_size_(max_task), thread_num_(thread_num)
    {
        running_.store(false, std::memory_order_relaxed);
    }

    size_t thread_num() const { return thread_num_; }

    void start();
    void end();

    bool add_task(const Task &task);
    bool add_task(Task &&task);
    bool started() const
    {
        return running_.load(std::memory_order_relaxed);
    }

  private:
    void run_in_thread();
    size_t task_size_;
    size_t thread_num_;

    std::atomic_bool running_;
    Mutex task_queue_mutex_;
    Mutex cond_mutex_;
    Condition compete_cond_;
    std::vector<std::jthread> threads_;
    std::deque<Task> tasks_;
};
} // namespace reactor

#endif