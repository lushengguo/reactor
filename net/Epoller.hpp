#pragma once
#ifndef REACTOR_EPOLLER_HPP
#define REACTOR_EPOLLER_HPP

#include "base/noncopyable.hpp"
#include "base/timestamp.hpp"
#include <map>
#include <unordered_map>
#include <vector>

struct epoll_event;
namespace reactor
{
class EventLoop;

class Poller : private noncopyable
{
  public:
    typedef std::vector<epoll_event>     epoll_events;
    typedef std::unordered_map<int, int> FdEventMap;
    Poller(EventLoop *loop);

    MicroTimeStamp epoll(MicroTimeStamp timeout);
    const epoll_events &active_events() const { return events_; }

    void remove_monitor_object(int fd);
    void new_monitor_object(int fd, int ievent);
    void modify_monitor_object(int fd, int ievent);

  private:
    constexpr static int NOEVENT = 0;

    int          epoll_fd_;
    EventLoop *  loop_;
    epoll_events events_;
    FdEventMap   feMap_;
};

} // namespace reactor
#endif