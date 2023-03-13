#include "parser.hpp"
#include <assert.h>
#include <iostream>
#include <regex>

namespace WebServer
{
Parser::ParseCode Parser::parse_request(std::string_view line) {}
Parser::ParseCode Parser::parse_header(std::string_view line) {}

Parser::ParseCode Parser::parse_body(std::string_view line, Parser::Request &request)
{
    std::string line1(line);
    std::regex re("(GET|POST|PUT)\\s+(/[a-z0-9A-Z/_]+|\\*)\\s*HTTP/(0.9|1.0|1.1)\\s*");
    std::smatch match;
    if (std::regex_match(line1, match, re))
    {
        assert(match.size() == 4);
        if (match.at(1).str() == "GET")
            request.rtype = RequestType::kGET;
        else if (match.at(1).str() == "POST")
            request.rtype = RequestType::kPOST;
        else if (match.at(1).str() == "PUT")
            request.rtype = RequestType::kPUT;

        request.path = match.at(2).str();

        if (match.at(3).str() == "0.9")
            request.version = HttpVersion::kHTTP0_9;
        else if (match.at(3).str() == "1.0")
            request.version = HttpVersion::kHTTP1_0;
        else if (match.at(3).str() == "1.1")
            request.version = HttpVersion::kHTTP1_1;

        return ParseCode::kOK;
    }
    else
    {
        return ParseCode::kBADLINE;
    }
}

} // namespace WebServer