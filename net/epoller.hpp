#pragma once

#include "base/noncopyable.hpp"
#include "net/Channel.hpp"
#include <map>

namespace reactor
{
class Poller : private noncopyable
{
  public:
    typedef std::map<int, Channel *> ChannelMap;
    Poller(EventLoop *loop);

    void update_channel(Channel *channel);
    void remove_channel(Channel *channel);

    mTimestamp epoll();

  private:
    size_t     max_event_;
    int        epoll_fd_;
    EventLoop *ploop_;
    ChannelMap channels_;
};

} // namespace reactor