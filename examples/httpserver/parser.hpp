#pragma once
#include <string_view>

namespace WebServer
{
class Parser
{
  public:
    enum ParseCode
    {
        kOk,
        kBADLINE
    };

    enum RequestType
    {
        kGET,
        kPOST,
        kPUT
    };

    enum HttpVersion
    {
        kHTTP0_9,
        kHTTP1_0,
        kHTTP1_1
    };

    struct Request
    {
        RequestType rtype;
        std::string path;
        HttpVersion version;
    } __attribute__(__packed__);

    ParseCode parse_request(std::string_view);
    ParseCode parse_header(std::string_view);
    ParseCode parse_body(std::string_view, Request &);
};
} // namespace WebServer