#pragma once

#include "base/noncopyable.hpp"
#include "net/Buffer.hpp"
#include "net/INetAddr.hpp"
#include <errno.h>
#include <string.h> //strerror

namespace reactor
{

class Socket : private noncopyable
{
  public:
    Socket(int backlog = default_backlog_);
    bool bind(const INetAddr &) const;
    bool listen() const;
    int  accept() const;
    int  connect(const INetAddr &) const;
    int  read(char *buffer, size_t max);
    int  write(char *buffer, size_t send_len);

    void set_tcp_nodelay();
    void set_nonblock();

  private:
    constexpr static int default_backlog_ = 1024;
    static int           backlog_;
    int                  fd_;
};

} // namespace reactor