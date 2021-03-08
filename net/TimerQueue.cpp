#include "net/EventLoop.hpp"
#include "net/TimerQueue.hpp"
#include <assert.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

namespace reactor
{
TimerQueue::TimerQueue(EventLoop *loop) : loop_(loop) {}

TimerQueue::TimerId TimerQueue::run_at(
  const TimeTaskCallback &cb, mTimestamp abs_mtime)
{
    if (abs_mtime < mtime())
        return -1;

    return create_timer_object(cb, 0, abs_mtime);
}

TimerQueue::TimerId TimerQueue::run_after(
  const TimeTaskCallback &cb, mTimestamp after)
{
    return run_at(cb, mtime() + after);
}

TimerQueue::TimerId TimerQueue::run_every(
  const TimeTaskCallback &cb, mTimestamp after, mTimestamp period)
{
    return create_timer_object(cb, period, after);
}

void TimerQueue::cancel(TimerQueue::TimerId id)
{
    if (timerMap_.count(id) == 1)
    {
        timerMap_.erase(id);
        loop_->remove_monitor_object(id);
    }
}

TimerQueue::TimerId TimerQueue::create_timer_object(
  const TimeTaskCallback &cb, mTimestamp period, mTimestamp abs_mtime)
{
    int fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    assert(fd > 0);
    itimerspec ispec;
    if (period != 0)
    {
        ispec.it_interval.tv_sec  = period / 1000000;
        ispec.it_interval.tv_nsec = (period % 1000000) * 1000;
    }
    else
    {
        ispec.it_interval.tv_sec  = 0;
        ispec.it_interval.tv_nsec = 0;
    }

    ispec.it_value.tv_sec  = abs_mtime / 1000000;
    ispec.it_value.tv_nsec = (abs_mtime % 1000000) * 1000;
    log_trace("new timer event will be called at %ld.%09lds, period=%ld.%09lds",
      ispec.it_value.tv_sec,
      ispec.it_value.tv_nsec,
      ispec.it_interval.tv_sec,
      ispec.it_interval.tv_nsec);

    if (timerfd_settime(fd, TFD_TIMER_ABSTIME, &ispec, nullptr) == -1)
    {
        log_error("timer fd set timer failed:%s", strerror(errno));
    }
    timerMap_[fd] = cb;
    loop_->new_monitor_object(static_cast<TimerId>(fd));

    return fd;
}

bool TimerQueue::period_timer_task(TimerId id) const
{
    itimerspec ispec;
    int        r = timerfd_gettime(id, &ispec);
    if (r == -1)
    {
        log_error("get time of timer fd failed:%s", strerror(errno));
        return false;
    }

    return ispec.it_interval.tv_sec != 0 || ispec.it_interval.tv_nsec != 0;
}

void TimerQueue::handle_event(TimerId id, int event)
{
    loop_->assert_in_loop_thread();
    assert(timerMap_.count(id) == 1);
    if (!(event & EPOLLIN))
        return;

    //用户注册的回调如果有线程安全问题 怎么解决？
    loop_->run_in_queue(timerMap_.at(id));
    if (!period_timer_task(id))
        cancel(id);
}
} // namespace reactor