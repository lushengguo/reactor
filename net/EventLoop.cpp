#include "net/EventLoop.hpp"
#include <sys/epoll.h>

namespace reactor
{
void EventLoop::loop()
{
    if (looping_)
        return;

    looping_ = true;

    while (true)
    {
        mTimestamp            receive_time = poller_->epoll();
        Poller::epoll_events &events       = poller_->active_events();
        for (epoll_event &event : events)
        {
            if (connMap_.count(event.data.fd) == 1)
            {
                connMap_.at(event.data.fd)
                  .handle_event(event.events, receive_time);
            }
        }
    }
}

void EventLoop::update_connection(TcpConnectionPtr conn)
{
    if (conn->fd() == -1)
    {
        poller_->remove_connection();
    }
    else if (cmap.count(conn->fd()) == 1)
    {
        poller_->modify_connection();
    }
    else
    {
        connMap_.insert(std::make_pair(conn->fd(), conn));
        poller_->new_connection();
    }
}

} // namespace reactor