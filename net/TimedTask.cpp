#include "net/TimedTask.hpp"
#include "net/EventLoop.hpp"
#include <assert.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>
#include <unistd.h>

namespace reactor
{

TimedTaskManager::TimedTaskManager(EventLoop *loop) : loop_(loop) {}

int TimedTaskManager::run_at(const TimerTaskCallback &cb, MilliTimestamp milli_timestamp_since_epoch)
{
    MilliTimestamp now = get_milli_timestamp();

    if (milli_timestamp_since_epoch < now)
    {
        return -1;
    }
    else if (milli_timestamp_since_epoch == now)
    {
        loop_->run_in_work_thread(cb);
        return -1;
    }
    else
    {
        return run_after(cb, milli_timestamp_since_epoch - now);
    }
}

int TimedTaskManager::run_after(const TimerTaskCallback &cb, MilliTimestamp after)
{
    int id = create_TimerId();
    loop_->run_in_loop_thread(std::bind(&TimedTaskManager::create_timer_event, this, id, cb, 0, after));
    return id;
}

int TimedTaskManager::run_every(const TimerTaskCallback &cb, MilliTimestamp after, MilliTimestamp period)
{
    int id = create_TimerId();
    if (after == 0)
        loop_->run_in_loop_thread(cb);

    loop_->run_in_loop_thread(std::bind(&TimedTaskManager::create_timer_event, this, id, cb, period, after));
    return id;
}

void TimedTaskManager::cancel(int id)
{
    loop_->assert_in_loop_thread();

    if (timer_map_.count(id) != 1)
        return;

    timer_map_.erase(id);
    loop_->remove_monitor_object(id);
    close(id);
}

int TimedTaskManager::create_TimerId() const
{
    int id = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    assert(id > 0);
    return id;
}

int TimedTaskManager::create_timer_event(int id, const TimerTaskCallback &cb, MilliTimestamp period,
                                         MilliTimestamp after)
{
    loop_->assert_in_loop_thread();

    itimerspec ispec;
    ispec.it_interval.tv_sec = period / 1000;
    ispec.it_interval.tv_nsec = (period % 1000) * 1000'000;
    ispec.it_value.tv_sec = after / 1000;
    ispec.it_value.tv_nsec = (after % 1000) * 1000'000;
    log_trace("new timer event will be called after {}ms, period={}ms", after, period);

    if (timerfd_settime(id, 0, &ispec, nullptr) == -1)
    {
        log_error("timer id set timer failed:%s", strerror(errno));
    }

    timer_map_[id] = {period != 0, cb};
    loop_->new_monitor_object(static_cast<int>(id));

    return id;
}

bool TimedTaskManager::periodical(int id) const
{
    loop_->assert_in_loop_thread();
    return timer_map_.at(id).periodical_;
}

void TimedTaskManager::handle_event(int id, int event)
{
    loop_->assert_in_loop_thread();

    // 可能在触发前被取消了
    if (timer_map_.count(id) != 1)
        return;

    assert(event & EPOLLIN);

    size_t val;
    int r = ::read(id, &val, sizeof val);
    assert(r == sizeof val);

    TimerTaskCallback cb = timer_map_.at(id).cb;
    loop_->run_in_work_thread(std::move(cb));
    if (!periodical(id))
        loop_->run_in_loop_thread(std::bind(&TimedTaskManager::cancel, this, id));
}
} // namespace reactor