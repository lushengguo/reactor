#include "net/epoller.hpp"
#include <assert.h>
#include <sys/epoll.h>

namespace reactor
{

Poller::Poller(EventLoop *loop) : max_event_(0), ploop_(loop)
{
    epoll_fd_ = ::epoll_create();
    assert(epoll_fd_ > 0);
}

void Poller::update_channel(Channel *channel)
{
    assert(channel);
    if (channels_.count(channel->fd()) == 1)
    {
        channels_[channel->fd()] = channel;
        ::epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, channel->event());
    }
    else
    {
        ::epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, channel->event());
    }
}

void Poller::remove_channel(Channel *channel)
{
    assert(channel);
    channels_.erase(channel->fd());
    ::epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, channel->event());
}

mTimestamp Poller::epoll()
{
    epoll_event event[max_event_];
    int         ret = ::epoll_wait(epoll_fd_, event, max_event_, 5);
    mTimestamp  now = now();
    assert(ret > 0);
    for (size_t i = 0; i < ret; i++)
    { channels_.at(event[i].data)->set_active_event(event[i].events); }
    return now;
}

} // namespace reactor
