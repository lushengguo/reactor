#include "base/log.hpp"
#include "base/threadpoll.hpp"

using namespace reactor;
void ThreadPoll::run_in_thread()
{
  while (true)
  {
    compete_cond_.wait();
    LOG_TRACE("thread id=%ld wake up", pthread_self());
    task_queue_mutex_.lock();
    LOG_TRACE("thread id=%ld lock the queue mutex", pthread_self());
    Task task;
    if (!tasks_.empty())
    {
      task = std::move(tasks_.front());
      tasks_.pop_front();
      LOG_TRACE("thread id=%ld get the task", pthread_self());
    }
    task_queue_mutex_.unlock();
    LOG_TRACE("thread id=%ld unlock the queue mutex", pthread_self());
    if (task)
    {
      task();
    }
    LOG_TRACE("thread id=%ld call task finish", pthread_self());
  }
}
bool ThreadPoll::add_task(Task &&task)
{
  MutexLockGuard lock(&task_queue_mutex_);
  if (tasks_.size() >= task_size_)
  {
    LOG_DEBUG("pthreadpoll task queue is full");
    return false;
  }
  else
  {
    LOG_DEBUG("add task to pthreadpoll task queue ,call later");
    tasks_.push_back(std::move(task));
  }

  if (!tasks_.empty())
  {
    compete_cond_.broadcast();
  }

  return true;
}

bool ThreadPoll::add_task(Task &task)
{
  MutexLockGuard lock(&task_queue_mutex_);
  if (tasks_.size() >= task_size_)
  {
    LOG_DEBUG("pthreadpoll task queue is full");
    return false;
  }
  else
  {
    LOG_DEBUG("add task to pthreadpoll task queue ,call later");
    tasks_.push_back(std::move(task));
  }
  return true;
}

void ThreadPoll::start()
{
  for (size_t i = 0; i < thread_num_; i++)
  {
    threads_.emplace_back(new Thread(std::bind(&ThreadPoll::run_in_thread, this)));
    threads_[i]->start();
  }
  running_ = true;
}