#include "base/log.hpp"
#include "net/Epoller.hpp"
#include "net/EventLoop.hpp"
#include <assert.h>
#include <errno.h>
#include <sys/epoll.h>

namespace reactor
{

Poller::Poller(EventLoop *loop) : loop_(loop)
{
    epoll_fd_ = ::epoll_create1(0);
    assert(epoll_fd_ > 0);
}

mTimestamp Poller::epoll()
{
    if (events_.size() < feMap_.size())
    {
        events_.resize(feMap_.size());
    }

    if (feMap_.empty())
        return mtime();

    int        r   = ::epoll_wait(epoll_fd_, events_.data(), feMap_.size(), 1);
    mTimestamp now = mtime();
    if (r == -1)
    {
        log_error("epoll error:%s", strerror(errno));
    }
    else if (static_cast<size_t>(r) < events_.size())
    {
        events_[r].events = NOEVENT;
    }

    return now;
}

void Poller::remove_monitor_object(int fd)
{
    loop_->assert_in_loop_thread();
    assert(feMap_.count(fd) == 1);
    epoll_event e;
    e.data.fd = fd;
    e.events  = NOEVENT;
    feMap_.erase(fd);
    int r = epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, &e);
    if (r != 0)
    {
        log_warn("remove fd from poller failed:%s", strerror(errno));
    }
}

void Poller::new_monitor_object(int fd, int ievent)
{
    loop_->assert_in_loop_thread();
    assert(feMap_.count(fd) == 0);
    epoll_event e;
    e.data.fd  = fd;
    e.events   = ievent;
    feMap_[fd] = ievent;
    int r      = epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &e);
    if (r != 0)
    {
        log_warn("add fd to poller failed:%s", strerror(errno));
    }
}

void Poller::modify_monitor_object(int fd, int ievent)
{
    loop_->assert_in_loop_thread();
    assert(feMap_.count(fd) == 1);
    epoll_event e;
    e.data.fd = fd;
    e.events  = ievent;
    int r     = epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &e);
    if (r != 0)
    {
        log_warn("modify fd in poller failed:%s", strerror(errno));
    }
}

} // namespace reactor
