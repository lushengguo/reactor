#pragma once
#ifndef REACTOR_ASYNC_TASK_HPP
#define REACTOR_ASYNC_TASK_HPP

#include "base/noncopyable.hpp"
#include "base/timestamp.hpp"
#include <functional>
#include <unordered_map>
namespace reactor
{
class EventLoop;

class TimedTaskManager : private noncopyable
{
  public:
    typedef std::function<void()> TimerTaskCallback;
    typedef int TimerId;
    typedef struct
    {
        bool periodical_;
        TimerTaskCallback cb;
    } TimerTaskRecord;
    typedef std::unordered_map<TimerId, TimerTaskRecord> TimerMap;

    TimedTaskManager(EventLoop *loop);

    TimerId run_at(const TimerTaskCallback &, MicroTimeStamp abs_mtime);
    TimerId run_after(const TimerTaskCallback &, MicroTimeStamp after);
    TimerId run_every(const TimerTaskCallback &, MicroTimeStamp after, MicroTimeStamp period);
    void cancel(TimerId);

    bool contain(TimerId id) const { return timer_map_.count(id); }

    void handle_event(TimerId, int event);

  private:
    TimerId create_TimerId() const;
    bool periodical(TimerId id) const;
    TimerId create_timer_event(TimerId id, const TimerTaskCallback &cb, MicroTimeStamp period, MicroTimeStamp after);

  private:
    TimerMap timer_map_;
    EventLoop *loop_;
};
} // namespace reactor

#endif