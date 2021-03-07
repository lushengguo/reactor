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
    typedef std::function<void()> TimeTaskCallback;
    typedef int                   TimerId;

    typedef std::unordered_map<TimerId, TimeTaskCallback> TimerMap;

    TimerQueue(EventLoop *loop);

    TimerId run_at(const TimeTaskCallback &, mTimestamp abs_mtime);
    TimerId run_after(const TimeTaskCallback &, mTimestamp after);
    TimerId run_every(
      const TimeTaskCallback &, mTimestamp after, mTimestamp period);
    void cancel(TimerId);

    bool contain(TimerId id) const { return timerMap_.count(id); }

    void handle_event(TimerId);

  private:
    TimerId create_timer_object(
      const TimeTaskCallback &cb, mTimestamp period, mTimestamp after);

  private:
    EventLoop *loop_;
    TimerMap   timerMap_;
};
} // namespace reactor

#endif