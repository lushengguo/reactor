#pragma once

#include <arpa/inet.h>
#include <string>
#include <string_view>
namespace reactor
{
class INetAddr
{
  public:
    INetAddr(std::string_view ip, uint16_t port)
    {
        int r = inet_pton(AF_INET, ip.c_str(), &be_ip_);
        if (r != 1)
            be_ip_ = 0;

        be_port_ = htobe16(port);
    }

    // be = big endian
    uint32_t be_ip() const { return be_ip_; }
    uint16_t be_port() const { return be_port_; }

  private:
    uint32_t be_ip_;
    uint16_t be_port_;
};
} // namespace reactor