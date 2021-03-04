#pragma once
#ifndef REACTOR_EVENTLOOP_HPP
#define REACTOR_EVENTLOOP_HPP

#include "base/noncopyable.hpp"
#include "base/threadpool.hpp"
#include "base/timer.hpp"
#include "base/timestamp.hpp"
#include <functional>
#include <map>
#include <memory>
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

    EventLoop() : self_(pthread_self()) {}

    void loop();

    void assert_in_loop_thread() const { assert(self_ == pthread_self()); }

    // timer event
    TimerID run_at(mTimestamp t, const TimerCallback &cb);
    TimerID run_every(mTimestamp t, const TimerCallback &cb);
    TimerID run_after(mTimestamp t, const TimerCallback &cb);
    void    cancel_timer_event(TimerID id);

    void run_in_queue(const Task &task) { pool_.add_task(task); }
    void run_in_queue(Task &&task) { pool_.add_task(std::move(task)); }

    void update_connection(TcpConnectionPtr conn);
    void remove_connection(TcpConnectionPtr conn);

  private:
    Poller *      poller_;
    pthread_t     self_;
    bool          looping_;
    ThreadPool    pool_;
    ConnectionMap connMap_;
};
} // namespace reactor
#endif