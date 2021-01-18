#pragma once
#include "base/condition.hpp"
#include "base/thread.hpp"
#include <functional>
#include <queue>
namespace reactor
{
class ThreadPoll
{
public:
  typedef std::function<void()> Task;
  ThreadPoll(size_t max_task = 20, int thread_num = 10)
    : task_size_(max_task), thread_num_(thread_num)
  {
  }

  void start();
  bool add_task(Task &task);
  bool add_task(Task &&task);
  bool running() const { return running_; }

  void run_in_thread();

private:
  size_t task_size_;
  size_t thread_num_;

  Mutex                 task_queue_mutex_;
  Mutex                 cond_mutex_;
  Condition             compete_cond_;
  std::vector<Thread *> threads_;
  std::deque<Task>      tasks_;
  bool                  running_;
};
} // namespace reactor