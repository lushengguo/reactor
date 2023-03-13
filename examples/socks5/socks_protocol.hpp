#pragma once
#ifndef SOCKS_PROTOCOL_HPP
#define SOCKS_PROTOCOL_HPP
#include <cstdint>
namespace socks5
{
enum class Method
{
    kNoAuthenticationRequired = 0x0,
    kGssApi = 0x1,
    kUsernamePassword = 0x2,
    kIANAAssigned = 0x3,
    kReservedForPrivateMethods = 0x80,
    kNoAcceptableMethods = 0xff
};

struct __attribute__((__packed__)) ClientHandshake
{
    uint8_t ver;
    uint8_t nmethods;
    char methods[0];
};

struct __attribute__((__packed__)) ServerHandshake
{
    uint8_t ver;
    uint8_t method;
};

struct __attribute__((__packed__)) Request
{
    uint8_t ver;
    uint8_t cmd;
    uint8_t rsv;
    uint8_t atyp;
    uint8_t addr[0];
};

enum class Cmd
{
    kConnect = 0x01,
    kBind = 0x02,
    kUdp = 0x03
};

enum class AddressType
{
    kIpv4 = 0x01,
    kDomainname = 0x03,
    kIpv6 = 0x04
};

struct __attribute__((__packed__)) Reply
{
    uint8_t ver;
    uint8_t rep;
    uint8_t rsv;
    uint8_t atyp;
    uint8_t addr[0];
};

enum class ReplyField
{
    kSucceeded = 0x00,
    kGeneralSocksServerFailure = 0x01,
    kConnectionNotAllowedByRuleSet = 0x02,
    kNetworkUnreachable = 0x03,
    kHostUnreachable = 0x04,
    kConnectionRefused = 0x05,
    kTTL_Expired = 0x06,
    kCommandNotSupported = 0x07,
    kAddressTypeNotSupported = 0x08,
    kUnassignedLowerBound = 0x09,
    kUnassignedUpperBound = 0xff,
};

} // namespace socks5
#endif