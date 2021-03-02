#include "base/Errno.hpp"
#include "base/log.hpp"
#include "net/Socket.hpp"
#include <arpa/inet.h>
#include <endian.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>

namespace reactor
{
Socket::Socket(int listen_queue_size)
{
    if (listen_queue_size <= 0)
        listen_queue_size_ = default_listen_queue_size_;
    else
        listen_queue_size_ = listen_queue_size;

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
    saddr.sin_port        = htobe16(addr.port());
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

    if (::listen(fd_, listen_queue_size_) == -1)
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

int Socket::read(int fd, char *buffer, size_t max)
{
    int ret = ::read(fd, buffer, max);
    if (ret < 0)
    {
        log_error("read from fd=%d error:%s", fd, strerror(errno));
    }
    return ret;
}

int Socket::write(int fd, char *buffer, size_t send_len)
{
    int ret = ::write(fd, buffer, send_len);
    if (ret < 0)
    {
        log_error("write to fd=%d error:%s", fd, strerror(errno));
    }
    else if (ret != send_len)
    {
        log_error("write to fd=%d incomplete, send %d ,expect %d",
          fd,
          ret,
          send_len);
    }
    return ret;
}

void Socket::read_into_buffer(int fd, Buffer &buffer, ErrorCode &err)
{
    char tmp[read_max_once_];
    int  ret = read(fd, tmp, read_max_once_);
    if (ret > 0)
    {
        buffer.append(tmp, ret);
    }
}

} // namespace reactor