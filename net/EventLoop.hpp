#pragma once
#ifndef REACTOR_EVENTLOOP_HPP
#define REACTOR_EVENTLOOP_HPP

#include "base/noncopyable.hpp"
#include "base/threadpool.hpp"
#include "base/timestamp.hpp"
#include "net/TimerQueue.hpp"
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace reactor
{

class Poller;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

//所有接口都是线程安全的 如果线程不安全程序会直接abort并且告知错误行为
//不会产生UB
class EventLoop : private noncopyable
{
  public:
    typedef std::function<void()>           Task;
    typedef std::map<int, TcpConnectionPtr> ConnectionMap;
    typedef std::deque<Task>                TaskQueue;
    using TimerId           = TimerQueue::TimerId;
    using TimerTaskCallback = TimerQueue::TimerTaskCallback;

    EventLoop();

    void loop();
    // abs_mtime如果小于当前时间则不执行
    TimerId run_at(const TimerTaskCallback &, mTimestamp abs_mtime);
    TimerId run_after(const TimerTaskCallback &, mTimestamp after);
    TimerId run_every(
      const TimerTaskCallback &, mTimestamp period, mTimestamp after);
    void cancel(TimerId);

    /*以下接口仅内部类调用 用户调用上面的接口*/
  public:
    //一些事件设计的要在EventLoop所在的线程跑
    //如果不在这个线程里跑(比如在线程池里)就有可能出现同步问题
    bool in_loop_thread() const;
    //在其他线程方法内调用该函数即代表该方法是线程安全的
    void assert_in_loop_thread() const;

    //对于监控事件的解决回调
    void run_in_work_thread(const Task &task);
    void run_in_work_thread(Task &&task);
    void run_in_loop_thread(const Task &func);
    void run_in_loop_thread(Task &&func);

    //新的监控事件
    void new_monitor_object(TimerId id);
    void remove_monitor_object(TimerId id);
    void update_monitor_object(TcpConnectionPtr conn);
    void remove_monitor_object(TcpConnectionPtr conn);

    void run_buffered_task();
    void handle_event(mTimestamp);

  private:
    Poller *        poller_;
    TimerQueue *    tqueue_;
    const pthread_t self_;
    bool            looping_;
    ThreadPool      pool_;
    ConnectionMap   connMap_;
    TaskQueue       task_queue_;
    Mutex           task_queue_mutex_;
};
} // namespace reactor
#endif