#pragma once
#include <strings.h>
#ifndef REACTOR_ENDPOINT_HPP
#define REACTOR_ENDPOINT_HPP

#include "base/log.hpp"
#include <arpa/inet.h>
#include <errno.h>
#include <string>
#include <string_view>
namespace reactor
{
class INetAddr
{
  public:
    INetAddr() : netip_(0), netport_(0) {}
    INetAddr(uint32_t ip, uint16_t port) : netip_(ip), netport_(port) {}
    INetAddr(std::string_view ip, uint16_t port)
    {
        std::string iip(ip);
        if (ip == "local_host")
            iip = "127.0.0.1";

        int r = inet_pton(AF_INET, iip.c_str(), &netip_);
        if (r != 1)
            netip_ = 0;

        netport_ = htobe16(port);
    }

    INetAddr(const INetAddr &rhs)
    {
        if (this != &rhs)
        {
            netip_ = rhs.netip_;
            netport_ = rhs.netport_;
        }
    }

    INetAddr &operator=(const INetAddr &rhs)
    {
        if (this != &rhs)
        {
            netip_ = rhs.netip_;
            netport_ = rhs.netport_;
        }
        return *this;
    }

    // problem
    std::string readable_ip() const
    {
        char ip[256];
        bzero(ip, 256);
        if (!inet_ntop(AF_INET, &netip_, ip, 256))
        {
            log_error("convert inet-addr to readable ip failed:%s", strerror(errno));
        }
        return std::string(ip);
    }

    std::string peer_addr() const
    {
        std::string res = readable_ip();
        if (res.empty())
            return res;

        return res.append(std::to_string(netport_));
    }

    uint16_t hostport() const { return be16toh(netport_); }
    uint32_t netip() const { return netip_; }
    uint16_t netport() const { return netport_; }

  private:
    uint32_t netip_;
    uint16_t netport_;
};
} // namespace reactor
#endif