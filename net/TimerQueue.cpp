#include "net/EventLoop.hpp"
#include "net/TimerQueue.hpp"
#include <assert.h>
#include <sys/timerfd.h>

namespace reactor
{
TimerQueue::TimerQueue(EventLoop *loop) : loop_(loop) {}

TimerQueue::TimerId TimerQueue::run_at(
  const TimeTaskCallback &cb, mTimestamp abs_mtime)
{
    timespec now;
    if (clock_gettime(CLOCK_REALTIME, &now) == -1)
    {
        log_error("get local time failed:%s", strerror(errno));
        return -1;
    }

    if ((now.tv_sec * 1000000 + now.tv_nsec / 1000) > abs_mtime)
    {
        return -1;
    }

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
  const TimeTaskCallback &cb, mTimestamp period, mTimestamp after)
{
    int fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK | TFD_CLOEXEC);
    assert(fd > 0);
    timespec now;
    if (clock_gettime(CLOCK_REALTIME, &now) == -1)
    {
        log_error("get local time failed:%s", strerror(errno));
    }

    itimerspec ispec;
    if (period != 0)
    {
        ispec.it_interval.tv_sec  = period / 1000000;
        ispec.it_interval.tv_nsec = period % 1000000;
    }
    else
    {
        ispec.it_interval.tv_sec  = 0;
        ispec.it_interval.tv_nsec = 0;
    }

    int forward_bit       = ((after % 1000000 + now.tv_nsec) > 1000000) ? 1 : 0;
    ispec.it_value.tv_sec = now.tv_sec + after % 1000000 + forward_bit;
    ispec.it_value.tv_nsec = (now.tv_nsec + after) % 1000000;

    int r = timerfd_settime(fd, TFD_TIMER_ABSTIME, &ispec, nullptr);
    if (r == -1)
    {
        log_error("timer fd set timer failed:%s", strerror(errno));
    }
    timerMap_.insert(std::make_pair(fd, cb));
    loop_->new_monitor_object(fd);

    return fd;
}

void TimerQueue::handle_event(TimerId id)
{
    loop_->assert_in_loop_thread();
    assert(timerMap_.count(id) == 1);

    //用户注册的回调如果有线程安全问题 怎么解决？
    loop_->run_in_queue(timerMap_.at(id));
}
} // namespace reactor