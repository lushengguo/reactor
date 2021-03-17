#pragma once
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
    INetAddr() : neip_(0), neport_(0) {}
    INetAddr(uint32_t ip, uint16_t port) : neip_(ip), neport_(port) {}
    INetAddr(std::string_view ip, uint16_t port) : ip_(ip)
    {
        std::string iip(ip);
        if (ip == "local_host")
            iip = "127.0.0.1";

        int r = inet_pton(AF_INET, iip.c_str(), &neip_);
        if (r != 1)
            neip_ = 0;

        neport_ = htobe16(port);
    }

    INetAddr(const INetAddr &rhs)
    {
        if (this != &rhs)
        {
            ip_     = rhs.ip_;
            neip_   = rhs.neip_;
            neport_ = rhs.neport_;
        }
    }

    INetAddr &operator=(const INetAddr &rhs)
    {
        if (this != &rhs)
        {
            ip_     = rhs.ip_;
            neip_   = rhs.neip_;
            neport_ = rhs.neport_;
        }
        return *this;
    }

    // problem
    std::string_view readable_ip() const
    {
        if (ip_.empty())
        {
            char ip[256];

            if (inet_ntop(AF_INET, &neip_, ip, 256))
            {
                ip_ = ip;
            }
            else
            {
                log_error("convert inet-addr to readable ip failed:%s",
                  strerror(errno));
            }
        }
        return ip_;
    }

    uint16_t hostport() const { return be16toh(neport_); }
    uint32_t netip() const { return neip_; }
    uint16_t netport() const { return neport_; }

  private:
    mutable std::string ip_;
    uint32_t            neip_;
    uint16_t            neport_;
};
} // namespace reactor
#endif