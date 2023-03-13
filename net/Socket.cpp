#include "net/Socket.hpp"
#include "base/Errno.hpp"
#include "base/log.hpp"
#include <arpa/inet.h>
#include <endian.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string_view>
#include <strings.h>
#include <sys/socket.h>

namespace reactor
{

Socket::Socket(Socket &&rhs)
{
    if (this != &rhs)
    {
        fd_ = -1;
        std::swap(fd_, rhs.fd_);
    }
}

// private func for accept
Socket::Socket(int fd, INetAddr &&addr) : self_endpoint_(std::move(addr)), backlog_(default_backlog_), fd_(fd) {}

Socket::~Socket() { close(); }

Socket::Socket(int backlog) : fd_(-1)
{
    if (backlog <= 0)
        backlog_ = default_backlog_;
    else
        backlog_ = backlog;
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
}

bool Socket::bind(const INetAddr &addr)
{
    self_endpoint_ = addr;
    if (fd_ <= 0)
    {
        log_error("bind port=%d failed: bad socket", self_endpoint_.hostport());
        return false;
    }

    sockaddr_in paddr;
    bzero(&paddr, sizeof paddr);
    paddr.sin_family = AF_INET;
    paddr.sin_port = addr.netport();
    paddr.sin_addr.s_addr = htobe64(INADDR_ANY);
    int r = ::bind(fd_, (const sockaddr *)&paddr, sizeof paddr);
    if (r == -1)
    {
        log_error("bind port=%d on socket=%d failed: %s", self_endpoint_.hostport(), fd_, strerror(errno));
        return false;
    }
    return true;
}

bool Socket::listen() const
{
    if (fd_ <= 0)
    {
        log_error("listen on port=%d failed: bad socket", self_endpoint_.hostport());
        return -1;
    }

    if (::listen(fd_, backlog_) == -1)
    {
        log_error("listen on socket=%d failed: %s", fd_, strerror(errno));
        return false;
    }
    return true;
}

Socket *Socket::accept() const
{
    if (fd_ <= 0)
    {
        log_error("accept on socket=%d port=%d failed: bad socket", fd_, self_endpoint_.hostport());
        return nullptr;
    }

    sockaddr_in paddr;
    socklen_t size = sizeof paddr;
    int fd = ::accept(fd_, (sockaddr *)&paddr, &size);
    if (fd == -1)
    {
        log_error("accept on socket=%d failed: %s", fd_, strerror(errno));
        return nullptr;
    }
    INetAddr addr(paddr.sin_addr.s_addr, paddr.sin_port);
    return new Socket(fd, std::move(addr));
}

bool Socket::connect(const INetAddr &addr) const
{
    if (fd_ <= 0)
    {
        log_error("connect ip=%s port=%d failed: bad socket", addr.readable_ip().data(), self_endpoint_.hostport());
        return -1;
    }

    sockaddr_in server;
    bzero(&server, sizeof server);
    server.sin_family = AF_INET;
    server.sin_port = addr.netport();
    server.sin_addr.s_addr = addr.netip();
    int r = ::connect(fd_, (sockaddr *)&server, sizeof server);
    if (r == -1)
    {
        log_error("connect ip=%s port=%d failed: %s", addr.readable_ip().data(), self_endpoint_.hostport(), strerror(errno));
    }
    return r == 0;
}

int Socket::read(char *buffer, size_t max) { return ::read(fd_, buffer, max); }

int Socket::write(const char *buffer, size_t send_len) { return ::write(fd_, buffer, send_len); }

int Socket::shutdown() { return ::shutdown(fd_, SHUT_WR); }

void Socket::close()
{
    if (fd_ > 0)
    {
        int r = ::close(fd_);
        if (r != 0)
        {
            log_warn("close file descriptor failed,fd=%d:%s", fd_, strerror(errno));
        }
    }
    fd_ = -1;
}

void Socket::set_nonblock()
{
    if (fd_ <= 0)
        log_error("set nonblocking error due to fd<0");

    int flags = fcntl(fd_, F_GETFL, 0);
    int r = fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
    if (r != 0)
        log_error("set fd nonblock error:%s", strerror(errno));
}

void Socket::set_tcp_nodelay()
{
    if (fd_ <= 0)
        log_error("set nonblocking error due to fd<0");

    int yes = 1;
    int r = setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof yes);
    if (r != 0)
        log_error("set fd tcp-nodelay error:%s", strerror(errno));
}

void Socket::set_reuse_addr()
{
    if (fd_ <= 0)
        log_error("set reuse-addr error due to fd<0");

    int yes = 1;
    int r = setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (char *)&yes, sizeof yes);
    if (r != 0)
        log_error("set socket resuse-addr error:%s", strerror(errno));
}
} // namespace reactor