#pragma once

#include "base/noncopyable"
#include "base/threadpool.hpp"
#include "base/timer.hpp"
#include "base/timestamp.hpp"
#include "net/Epoller.hpp"
#include "net/TcpConnection.hpp"
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace reactor
{
class EventLoop : private noncopyable
{
  public:
    typedef std::function<void()>           Task;
    typedef std::function<void()>           TimerCallback;
    typedef size_t                          TimerID;
    typedef std::map<int, TcpConnectionPtr> ConnectionMap;
    typedef std::map<int, TcpServerPtr>     ServerMap;
    typedef std::map<int, TcpClientPtr>     ClientMap;

    void loop();

    // timer event
    TimerID run_at(mTimestamp t, const TimerCallback &cb);
    TimerID run_every(mTimestamp t, const TimerCallback &cb);
    TimerID run_after(mTimestamp t, const TimerCallback &cb);
    void    cancel_timer_event(TimerID id);

    void run_task(const Task &task) { pool_.add_task(task); }

    void update_connection(TcpConnectionPtr conn);

  private:
    std::unique_ptr<Poller> poller_;

    bool          looping_;
    ThreadPool    pool_;
    ConnectionMap connMap_;
    ServerMap     serverMap_; // only wait for accept event
    ClientMap     clientMap_; // only wait for connect event
};
} // namespace reactor