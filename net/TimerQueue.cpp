#include "net/EventLoop.hpp"
#include "net/TimerQueue.hpp"
#include <assert.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace reactor
{

TimerQueue::TimerQueue(EventLoop *loop) : loop_(loop) {}

TimerQueue::TimerId TimerQueue::run_at(const TimerTaskCallback &cb, MicroTimeStamp abs_mtime)
{
    MicroTimeStamp now = micro_timestamp();

    if (abs_mtime < now)
    {
        return -1;
    }
    else if (abs_mtime == now)
    {
        loop_->run_in_work_thread(cb);
        return -1;
    }
    else
    {
        return run_after(cb, abs_mtime - now);
    }
}

TimerQueue::TimerId TimerQueue::run_after(const TimerTaskCallback &cb, MicroTimeStamp after)
{
    TimerId id = create_TimerId();
    loop_->run_in_loop_thread(
      std::bind(&TimerQueue::create_timer_event, this, id, cb, 0, after));
    return id;
}

TimerQueue::TimerId TimerQueue::run_every(const TimerTaskCallback &cb, MicroTimeStamp after, MicroTimeStamp period)
{
    TimerId id = create_TimerId();
    loop_->run_in_loop_thread(
      std::bind(&TimerQueue::create_timer_event, this, id, cb, period, after));
    return id;
}

void TimerQueue::cancel(TimerQueue::TimerId id)
{
    loop_->assert_in_loop_thread();

    if (timerMap_.count(id) != 1)
        return;

    timerMap_.erase(id);
    loop_->remove_monitor_object(id);
    close(id);
}

TimerQueue::TimerId TimerQueue::create_TimerId() const
{
    int id = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    assert(id > 0);
    return id;
}

TimerQueue::TimerId TimerQueue::create_timer_event(TimerId id, const TimerTaskCallback &cb, MicroTimeStamp period, MicroTimeStamp after)
{
    loop_->assert_in_loop_thread();

    itimerspec ispec;
    ispec.it_interval.tv_sec  = period / 1000;
    ispec.it_interval.tv_nsec = (period % 1000) * 1000000;
    ispec.it_value.tv_sec     = after / 1000;
    ispec.it_value.tv_nsec    = (after % 1000) * 1000000;
    log_trace(
      "new timer event will be called after %ld.%09lds, period=%ld.%09lds",
      ispec.it_value.tv_sec,
      ispec.it_value.tv_nsec,
      ispec.it_interval.tv_sec,
      ispec.it_interval.tv_nsec);

    if (timerfd_settime(id, 0, &ispec, nullptr) == -1)
    {
        log_error("timer id set timer failed:%s", strerror(errno));
    }

    timerMap_[id] = {period != 0, cb};
    loop_->new_monitor_object(static_cast<TimerId>(id));

    return id;
}

bool TimerQueue::periodical(TimerId id) const
{
    loop_->assert_in_loop_thread();
    return timerMap_.at(id).periodical_;
}

void TimerQueue::handle_event(TimerId id, int event)
{
    loop_->assert_in_loop_thread();

    //可能在触发前被取消了
    if (timerMap_.count(id) != 1)
        return;

    assert(event & EPOLLIN);

    size_t val;
    int    r = ::read(id, &val, sizeof val);
    assert(r == sizeof val);

    TimerTaskCallback cb = timerMap_.at(id).cb;
    loop_->run_in_work_thread(std::move(cb));
    if (!periodical(id))
        loop_->run_in_loop_thread(std::bind(&TimerQueue::cancel, this, id));
}
} // namespace reactor