#pragma once

#include <string>
#include <string_view>

namespace reactor
{
class INetAddr
{
  public:
    INetAddr(std::string_view ip, uint16_t port) : ip_(ip), port_(port) {}

    const char *ip() const { return ip_.c_str(); }
    uint16_t    port() const { return port_; }

  private:
    std::string ip_;
    uint16_t    port_;
};
} // namespace reactor