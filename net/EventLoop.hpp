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

class EventLoop : private noncopyable
{
  public:
    typedef std::function<void()>           Task;
    typedef std::function<void()>           TimerCallback;
    typedef size_t                          TimerID;
    typedef std::map<int, TcpConnectionPtr> ConnectionMap;
    using TimerId          = TimerQueue::TimerId;
    using TimeTaskCallback = TimerQueue::TimeTaskCallback;

    EventLoop();

    //两步构造 不能在EventLoop的构造函数里把this暴露给Poller
    //但是有线程安全问题
    void init();

    void loop();

    //一些事件设计的要在EventLoop所在的线程跑
    //如果不在这个线程里跑(比如在线程池里)就有可能出现同步问题
    void assert_in_loop_thread() const;

    // abs_mtime如果小于当前时间则不执行
    TimerId run_at(const TimeTaskCallback &, mTimestamp abs_mtime);
    TimerId run_after(const TimeTaskCallback &, mTimestamp after);
    TimerId run_every(
      const TimeTaskCallback &, mTimestamp period, mTimestamp after);
    void cancel(TimerId);

    void run_in_queue(const Task &task) { pool_.add_task(task); }
    void run_in_queue(Task &&task) { pool_.add_task(std::move(task)); }

  public:
    //用户不要调用这些
    void new_monitor_object(TimerID id);
    void remove_monitor_object(TimerID id);

    void update_monitor_object(TcpConnectionPtr conn);
    void remove_monitor_object(TcpConnectionPtr conn);

  private:
    Poller *      poller_;
    TimerQueue *  tqueue_;
    pthread_t     self_;
    bool          looping_;
    ThreadPool    pool_;
    ConnectionMap connMap_;
};
} // namespace reactor
#endif