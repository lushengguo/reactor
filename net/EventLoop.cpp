#include "net/Epoller.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpConnection.hpp"
#include <sys/epoll.h>

namespace reactor
{

EventLoop::EventLoop() : self_(pthread_self()), looping_(false) {}

void EventLoop::init_poller() { poller_ = new Poller(this); }

void EventLoop::loop()
{
    // not thread safe
    if (looping_)
        return;

    looping_ = true;
    assert(poller_);

    while (true)
    {
        mTimestamp receive_time = poller_->epoll();

        const Poller::epoll_events &events = poller_->active_events();
        for (const epoll_event &event : events)
        {
            if (event.events == 0)
            {
                break;
            }

            log_trace("event fd=%d", event.data.fd);
            assert(connMap_.count(event.data.fd) == 1);
            connMap_.at(event.data.fd)
              ->handle_event(event.events, receive_time);
        }
    }
}

void EventLoop::update_connection(TcpConnectionPtr conn)
{
    if (connMap_.count(conn->fd()) == 1)
    {
        poller_->modify_connection(conn->fd(), conn->interest_event());
    }
    else
    {
        connMap_.insert(std::make_pair(conn->fd(), conn));
        poller_->new_connection(conn->fd(), conn->interest_event());
    }
}

void EventLoop::remove_connection(TcpConnectionPtr conn)
{
    assert(connMap_.count(conn->fd()) == 1);
    poller_->remove_connection(conn->fd());
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

} // namespace reactor