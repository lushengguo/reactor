#include "net/EventLoop.hpp"

namespace reactor
{
void EventLoop::loop()
{
    if (looping_)
        return;

    while (true)
    {
        mTimestamp now = poller_->epoll();
        for (auto &channel : channels_) { channel.second->handle_event(now); }
    }
}

void update_channel(Channel *channel)
{
    if (channel)
    {
        channels_[channel->fd()] = channel;
    }
}

} // namespace reactor