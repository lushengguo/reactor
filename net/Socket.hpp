#pragma once
#ifndef REACTOR_SOCKET_HPP
#define REACTOR_SOCKET_HPP

#include "base/Buffer.hpp"
#include "base/Errno.hpp"
#include "base/noncopyable.hpp"
#include "net/INetAddr.hpp"
#include <errno.h>
#include <string.h> //strerror

namespace reactor
{

class Socket : private noncopyable
{
  public:
    Socket(int backlog = default_backlog_);
    Socket(Socket &&rhs);
    ~Socket();

    bool    bind(const INetAddr &);
    bool    listen() const;
    Socket *accept() const;
    bool    connect(const INetAddr &) const;
    int     read(char *buffer, size_t max);
    int     write(const char *buffer, size_t send_len);

    void set_tcp_nodelay();
    void set_nonblock();
    void set_reuse_addr();

    int shutdown();
    int fd() const { return fd_; }

    std::string readable_ip() { return self_endpoint_.readable_ip(); }
    std::string peer_addr() { return self_endpoint_.peer_addr(); }
    uint16_t         hostport() { return self_endpoint_.hostport(); }

  private:
    // for return new connection info
    Socket(int fd, INetAddr &&addr);
    void close();

    constexpr static int default_backlog_ = 1024;

    INetAddr self_endpoint_;
    int      backlog_;
    int      fd_;
};

} // namespace reactor
#endif