#pragma once
#include <limits>
#ifndef REACTOR_EVENTLOOP_HPP
#define REACTOR_EVENTLOOP_HPP

#include "base/noncopyable.hpp"
#include "base/threadpool.hpp"
#include "base/timestamp.hpp"
#include "net/Epoller.hpp"
#include "net/TimedTask.hpp"
#include <functional>
#include <map>
#include <memory>
#include <set>
#include <vector>

namespace reactor
{

class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
typedef std::weak_ptr<TcpConnection> TcpConnectionWeakPtr;

//所有接口都是线程安全的 如果线程不安全程序会直接abort并且告知错误行为
//不会产生UB
class EventLoop : private noncopyable
{
  public:
    typedef std::function<void()> Task;
    typedef std::map<int, TcpConnectionPtr> ConnectionMap;
    typedef std::deque<Task> TaskQueue;
    using TimerTaskCallback = TimedTaskManager::TimerTaskCallback;

    EventLoop();
    ~EventLoop();

    void loop(MilliTimestamp break_time = std::numeric_limits<MilliTimestamp>::max());
    // abs_mtime如果小于当前时间则不执行
    int run_at(const TimerTaskCallback &, MilliTimestamp abs_mtime);
    int run_after(const TimerTaskCallback &, MilliTimestamp after);
    int run_every(const TimerTaskCallback &, MilliTimestamp period, MilliTimestamp after);
    void cancel(int);

    /*以下接口仅内部类调用 用户调用上面的接口*/
  public:
    // 一些事件设计的要在EventLoop所在的线程跑
    // 如果不在这个线程里跑(比如在线程池里)就有可能出现同步问题
    bool in_loop_thread() const;
    // 在其他线程方法内调用该函数即代表该方法是线程安全的
    void assert_in_loop_thread() const;

    // 对于监控事件的解决回调
    void run_in_work_thread(const Task &task);
    void run_in_work_thread(Task &&task);
    void run_in_loop_thread(const Task &func);
    void run_in_loop_thread(Task &&func);

    // 新的监控事件
    void new_monitor_object(int);
    void remove_monitor_object(int);
    void update_monitor_object(TcpConnectionPtr);
    void remove_monitor_object(TcpConnectionPtr);

    void run_buffered_task();
    void handle_event(MilliTimestamp);

  private:
    Poller poller_;
    TimedTaskManager *tqueue_;
    pthread_t self_;
    bool looping_;
    ThreadPool pool_;
    ConnectionMap connMap_;
    TaskQueue task_queue_;
    Mutex task_queue_mutex_;
};
} // namespace reactor
#endif