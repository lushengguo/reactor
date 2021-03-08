#include "net/Epoller.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpConnection.hpp"
#include <sys/epoll.h>

namespace reactor
{

EventLoop::EventLoop()
  : poller_(nullptr), tqueue_(nullptr), self_(pthread_self()), looping_(false)
{}

void EventLoop::init()
{
    if (poller_ == nullptr && tqueue_ == nullptr)
    {
        poller_ = new Poller(this);
        tqueue_ = new TimerQueue(this);
    }

    pool_.start();
}

void EventLoop::loop()
{
    // not thread safe
    if (looping_)
        return;

    looping_ = true;
    while (true)
    {
        mTimestamp receive_time = poller_->epoll();
        auto       events       = poller_->active_events();
        for (const epoll_event &event : events)
        {
            // events的size等于当前poller监听描述符的数量
            // poller返回的events会把最后一个活跃的event的下一个event的events设为0
            // 如果存在的话
            if (event.events == 0)
                break;

            //判断是否是定时事件
            if (tqueue_->contain(event.data.fd))
            {
                tqueue_->handle_event(event.data.fd, event.events);
                continue;
            }

            //连接事件
            if (connMap_.count(event.data.fd) == 1)
            {
                connMap_.at(event.data.fd)
                  ->handle_event(event.events, receive_time);
            }
        }
    }
}

void EventLoop::update_monitor_object(TcpConnectionPtr conn)
{
    if (connMap_.count(conn->fd()) == 1)
    {
        poller_->modify_monitor_object(conn->fd(), conn->interest_event());
    }
    else
    {
        connMap_.insert(std::make_pair(conn->fd(), conn));
        poller_->new_monitor_object(conn->fd(), conn->interest_event());
    }
}

void EventLoop::remove_monitor_object(TcpConnectionPtr conn)
{
    assert(connMap_.count(conn->fd()) == 1);
    poller_->remove_monitor_object(conn->fd());
    // connection的生命周期由EventLoop管理
    // erase后析构自动断开连接(前提是用户没有保存TcpConnectionPtr)
    connMap_.erase(conn->fd());
}

void EventLoop::assert_in_loop_thread() const
{
    if (self_ != pthread_self())
    {
        log_error("loop thread id=%d,call assert thread id=%d",
          self_,
          pthread_self());
        assert(false);
    }
}

EventLoop::TimerId EventLoop::run_at(
  const EventLoop::TimeTaskCallback &cb, mTimestamp abs_mtime)
{
    return tqueue_->run_at(cb, abs_mtime);
}

EventLoop::TimerId EventLoop::run_after(
  const EventLoop::TimeTaskCallback &cb, mTimestamp after)
{
    return tqueue_->run_after(cb, after);
}

EventLoop::TimerId EventLoop::run_every(
  const EventLoop::TimeTaskCallback &cb, mTimestamp period, mTimestamp after)
{
    return tqueue_->run_every(cb, period, after);
}

void EventLoop::cancel(EventLoop::TimerId id) { tqueue_->cancel(id); }

void EventLoop::new_monitor_object(EventLoop::TimerID id)
{
    poller_->new_monitor_object(id, EPOLLIN);
}

void EventLoop::remove_monitor_object(EventLoop::TimerID id)
{
    poller_->remove_monitor_object(id);
}

} // namespace reactor