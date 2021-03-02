// #include "../parser.hpp"
#include <iostream>
#include <regex>
#include <string>
#include <string_view>

// using namespace WebServer;

int main()
{
    std::string request1 = "GET /path/to/the/file HTTP/1.1 ";
    std::regex  re(
      "(GET|POST|PUT)\\s+([a-z0-9A-Z/_]*)\\s*HTTP/(0.9|1.0|1.1)\\s*");

    std::smatch match;
    if (std::regex_match(request1, match, re))
    {
        for (int i = 0; i < match.size(); i++)
        { std::cout << match[i].str() << std::endl; }
    }
}