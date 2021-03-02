#pragma once

#include "base/noncopyable.hpp"
#include "net/Buffer.hpp"
#include "net/INetAddr.hpp"

namespace reactor
{

class Socket : private noncopyable
{
  public:
    Socket(int listen_queue_size = default_listen_queue_size_);
    bool bind(const INetAddr &) const;
    bool listen() const;
    int  accept() const;
    int  connect(const INetAddr &addr) const;
    void set_tcp_nodelay();
    int  read(int fd, char *buffer, size_t max);
    int  write(int fd, char *buffer, size_t send_len);

    void read_into_Buffer(int fd, Buffer &buffer);

  private:
    constexpr static size_t read_max_once_             = 65535;
    constexpr static int    default_listen_queue_size_ = 1024;
    static int              listen_queue_size_;
    int                     fd_;
};

} // namespace reactor