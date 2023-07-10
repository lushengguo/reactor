#include "net/Epoller.hpp"
#include "base/log.hpp"
#include "net/EventLoop.hpp"
#include <assert.h>
#include <errno.h>

namespace reactor
{

Poller::Poller()
{
    epoll_fd_ = ::epoll_create1(0);
    assert(epoll_fd_ > 0);
}

MicroTimeStamp Poller::epoll(MicroTimeStamp timeout)
{
    if (events_.size() < event_map_.size())
    {
        events_.resize(event_map_.size());
    }

    if (event_map_.empty())
        return micro_timestamp();

    int r = ::epoll_wait(epoll_fd_, events_.data(), event_map_.size(), timeout);
    if (r < 0)
    {
        log_error("epoll error:%s, epoll_wait return %d, epolling %d fds", strerror(errno), r, events_.size());
    }
    else if (static_cast<size_t>(r) < events_.size())
    {
        events_[r].events = NOEVENT;
    }

    return micro_timestamp();
}

void Poller::remove_monitor_object(int fd)
{
    assert(event_map_.count(fd) == 1);
    epoll_event e;
    e.data.fd = fd;
    e.events = NOEVENT;
    event_map_.erase(fd);
    int r = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &e);
    if (r != 0)
    {
        log_warn("remove fd from poller failed:%s", strerror(errno));
    }
}

void Poller::new_monitor_object(int fd, int ievent)
{
    assert(event_map_.count(fd) == 0);
    epoll_event e;
    e.data.fd = fd;
    e.events = ievent;
    event_map_[fd] = ievent;
    int r = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &e);
    if (r != 0)
    {
        log_warn("add fd to poller failed,fd=%d,ievent=%d:%s", fd, ievent, strerror(errno));
    }
}

void Poller::modify_monitor_object(int fd, int ievent)
{
    assert(event_map_.count(fd) == 1);
    epoll_event e;
    e.data.fd = fd;
    e.events = ievent;
    int r = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &e);
    if (r != 0)
    {
        log_warn("modify fd in poller failed:%s", strerror(errno));
    }
}

} // namespace reactor
