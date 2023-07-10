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
    typedef struct
    {
        bool periodical_;
        TimerTaskCallback cb;
    } TimerTaskRecord;
    typedef std::unordered_map<int, TimerTaskRecord> TimerMap;

    TimedTaskManager(EventLoop *loop);

    // return fd of timer task
    int run_at(const TimerTaskCallback &, MilliTimestamp milli_timestamp_since_epoch);
    int run_after(const TimerTaskCallback &, MilliTimestamp after);
    int run_every(const TimerTaskCallback &, MilliTimestamp after, MilliTimestamp period);
    void cancel(int);

    bool contain(int id) const
    {
        return timer_map_.count(id);
    }

    void handle_event(int, int event);

  private:
    int create_TimerId() const;
    bool periodical(int id) const;
    int create_timer_event(int id, const TimerTaskCallback &cb, MilliTimestamp period, MilliTimestamp after);

  private:
    TimerMap timer_map_;
    EventLoop *loop_;
};
} // namespace reactor

#endif