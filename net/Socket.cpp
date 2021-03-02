#include "base/Errno.hpp"
#include "base/log.hpp"
#include "net/Socket.hpp"
#include <arpa/inet.h>
#include <endian.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <strings.h>
#include <sys/socket.h>

namespace reactor
{
Socket::Socket(int backlog)
{
    if (backlog <= 0)
        backlog_ = default_backlog_;
    else
        backlog_ = backlog;

    fd_ = socket(AF_INET, SOCK_STREAM, 0);
}

bool Socket::bind(const INetAddr &addr) const
{
    if (fd_ <= 0)
    {
        log_error("bind port=%d failed: bad socket", addr.port());
        return false;
    }

    sockaddr_in saddr;
    bzero(&saddr, sizeof saddr);
    saddr.sin_family      = AF_INET;
    saddr.sin_port        = addr.be_port();
    saddr.sin_addr.s_addr = htobe64(INADDR_ANY);
    int ret               = ::bind(fd_, (const sockaddr *)&saddr, sizeof saddr);
    if (ret == -1)
    {
        log_error("bind port=%d on socket=%d failed: %s",
          addr.port(),
          fd_,
          strerror(errno));
        return false;
    }
    return true;
}

bool Socket::listen() const
{
    if (fd_ <= 0)
    {
        log_error("listen on socket=%d failed: bad socket", fd_, addr.port());
        return -1;
    }

    if (::listen(fd_, backlog_) == -1)
    {
        log_error("listen on socket=%d failed: %s", fd_, strerror(errno));
        return false;
    }
    return true;
}

int Socket::accept() const
{
    if (fd_ <= 0)
    {
        log_error("accept on socket=%d failed: bad socket", fd_, addr.port());
        return -1;
    }

    sockaddr_in saddr;
    socklen_t   size = sizeof saddr;
    int         ret  = ::accept(fd_, (sockaddr *)&saddr, &size);
    if (ret == -1)
    {
        log_error("accept on socket=%d failed: %s", fd_, strerror(errno));
    }
    return ret;
}

int Socket::connect(const INetAddr &addr) const
{
    if (fd_ <= 0)
    {
        log_error("connect ip=%s port=%d failed: bad socket",
          addr.ip(),
          addr.port());
        return -1;
    }

    sockaddr_in server;
    bzero(&server, sizeof server);
    server.sin_family = AF_INET;
    server.sin_port   = htobe16(addr.port());
    inet_pton(AF_INET, addr.ip(), &server.sin_addr);
    int ret = ::connect(fd_, (sockaddr *)&server, sizeof server);
    if (ret == -1)
    {
        log_error("connect ip=%s port=%d failed: %s",
          addr.ip(),
          addr.port(),
          strerror(errno));
    }
    return ret;
}

int Socket::read(char *buffer, size_t max) { return ::read(fd_, buffer, max); }

int Socket::write(char *buffer, size_t send_len)
{
    return ::write(fd_, buffer, send_len);
}

void Socket::close()
{
    if (fd_ > 0)
    {
        int r = ::close(fd_);
        if (r != 0)
        {
            log_warn("close file descriptor failed:%s", strerror(errno));
        }
    }
    fd_ = -1;
}

void Socket::set_nonblock()
{
    if (fd <= 0)
        log_error("set nonblocking error due to fd<0");

    int flags = fcntl(fd_, F_GETFL, 0);
    int r     = fcntl(fd_, F_SETFL, flags | O_NONBLOCK);
    if (r != 0)
        log_error("set fd nonblock error:%s", strerror(errno));
}

void Socket::set_tcp_no_delay()
{
    if (fd <= 0)
        log_error("set nonblocking error due to fd<0");

    int yes = 1;
    int r = setsockopt(fd_, IPPROTO_TCP, TCP_NODELAY, (char *)&yes, sizeof yes);
    if (r != 0)
        log_error("set fd tcp-nodelay error:%s", strerror(errno));
}

} // namespace reactor