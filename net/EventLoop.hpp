#pragma once

#include "base/noncopyable"
#include "base/threadpool.hpp"
#include "base/timer.hpp"
#include "base/timestamp.hpp"
#include "net/Channel.hpp"
#include "net/epoller.hpp"
#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace reactor
{
class EventLoop : private noncopyable
{
  public:
    typedef std::function<void()> Task;
    typedef std::function<void()> TimerCallback;
    typedef size_t                TimerID;

    void loop();

    // timer event
    TimerID run_at(mTimestamp t, const TimerCallback &cb);
    TimerID run_every(mTimestamp t, const TimerCallback &cb);
    TimerID run_after(mTimestamp t, const TimerCallback &cb);
    void    cancel_timer_event(TimerID id);

    void run_task(const Task &task) { pool_.add_task(task); }

    void update_connection(TcpConnectionPtr conn);

  private:
    bool                    looping_;
    std::unique_ptr<Poller> poller_;
    ThreadPool              pool_;
};
} // namespace reactor