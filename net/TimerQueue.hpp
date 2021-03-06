#pragma once
#ifndef REACTOR_TIMERQUEUE_HPP
#define REACTOR_TIMERQUEUE_HPP

#include "base/noncopyable.hpp"
#include "base/timestamp.hpp"
#include <functional>
#include <unordered_map>
namespace reactor
{
class EventLoop;

class TimerQueue : private noncopyable
{
  public:
    typedef std::function<void()> TimerTaskCallback;
    typedef int                   TimerId;
    typedef struct
    {
        bool              periodical_;
        TimerTaskCallback cb;
    } TimerTaskRecord;
    typedef std::unordered_map<TimerId, TimerTaskRecord> TimerMap;

    TimerQueue(EventLoop *loop);

    TimerId run_at(const TimerTaskCallback &, mTimestamp abs_mtime);
    TimerId run_after(const TimerTaskCallback &, mTimestamp after);
    TimerId run_every(
      const TimerTaskCallback &, mTimestamp after, mTimestamp period);
    void cancel(TimerId);

    bool contain(TimerId id) const { return timerMap_.count(id); }

    void handle_event(TimerId, int event);

  private:
    TimerId create_TimerId() const;
    bool    periodical(TimerId id) const;
    TimerId create_timer_event(TimerId id,
      const TimerTaskCallback &        cb,
      mTimestamp                       period,
      mTimestamp                       after);

  private:
    TimerMap   timerMap_;
    EventLoop *loop_;
};
} // namespace reactor

#endif