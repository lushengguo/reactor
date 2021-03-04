#include "net/Epoller.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpConnection.hpp"
#include <sys/epoll.h>

namespace reactor
{
void EventLoop::loop()
{
    if (looping_)
        return;

    // not thread safe
    looping_ = true;
    assert(!poller_);
    while (true)
    {
        mTimestamp                  receive_time = poller_->epoll();
        const Poller::epoll_events &events       = poller_->active_events();
        for (const epoll_event &event : events)
        {
            if (event.events == 0)
            {
                break;
            }

            if (connMap_.count(event.data.fd) == 1)
            {
                connMap_.at(event.data.fd)
                  ->handle_event(event.events, receive_time);
            }
        }
    }
}

void EventLoop::update_connection(TcpConnectionPtr conn)
{
    if (conn->fd() == -1)
    {
        poller_->remove_connection(conn->fd());
    }
    else if (connMap_.count(conn->fd()) == 1)
    {
        poller_->modify_connection(conn->fd(), conn->interest_event());
    }
    else
    {
        connMap_.insert(std::make_pair(conn->fd(), conn));
        poller_->new_connection(conn->fd(), conn->interest_event());
    }
}

} // namespace reactor