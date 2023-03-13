#include "protocol.hpp"
#include "base/log.hpp"
#include "base/mixed.hpp"
#include <assert.h>
#include <iostream>
#include <map>
#include <regex>
namespace reactor
{
size_t RedisProtocol::parse_response(const std::string &frame, std::string &result)
{
    if (frame.empty())
        return 0;

    { // debug
        std::string frame_debug(frame);
        frame_debug = replace_all(frame_debug, "\n", "\\n");
        frame_debug = replace_all(frame_debug, "\r", "\\r");
        // log_debug("receive raw response:%s", frame_debug.c_str());
    }

    //找到一个ack的起始位置
    ResponseType type;
    int header_index = find_header(frame, type);
    if (header_index == -1)
    {
        // can not find header should retrieve all
        return frame.size();
    }

    // 解析不完全有两种可能：
    // 1.报文不完整/丢包 这个暂时不考虑 - - 比较复杂
    // 2.只收到了一部分ack 等待一整个报文到达后返回response
    size_t response_len = generic_parse(frame.substr(header_index), result, type);
    return response_len + header_index;
}

size_t RedisProtocol::generic_parse(const std::string &frame, std::string &result, ResponseType type)
{
    switch (type)
    {
    case kSimpleStringItem:
        return parse_simple_string_item(frame, result);
    case kArrayItem:
        return parse_array_item(frame, result);
    case kErrString:
        return parse_err_string(frame, result);
    case kNumberItem:
        return parse_number_item(frame, result);
    case kBulkString:
        return parse_bulk_string(frame, result);
    default:
        return 0;
    }
}

int RedisProtocol::find_header(const std::string &frame, ResponseType &type)
{
    int index = 0;
    for (auto ch : frame)
    {
        if (ch == '+')
        {
            type = kSimpleStringItem;
            return index;
        }
        else if (ch == '-')
        {
            type = kErrString;
            return index;
        }
        else if (ch == '*')
        {
            type = kArrayItem;
            return index;
        }
        else if (ch == ':')
        {
            type = kNumberItem;
            return index;
        }
        else if (ch == '$')
        {
            type = kBulkString;
            return index;
        }
        index++;
    }

    return -1;
}

size_t RedisProtocol::parse_simple_string_item(const std::string &frame, std::string &result)
{
    std::regex match("\\+([a-zA-Z]+)\r\n");
    std::smatch regex_result;
    if (std::regex_match(frame, regex_result, match))
    {
        result = regex_result[1].str();
        return 3 + result.size(); // 3 : header + CRLF
    }

    return 0;
}

size_t RedisProtocol::parse_array_item(const std::string &frame, std::string &result)
{
    size_t first_crlf = frame.find("\r\n");
    if (first_crlf == std::string::npos)
        return 0;

    // parse array size
    std::string array_size_string = frame.substr(1, first_crlf);
    assert(is_numeric_string(array_size_string));
    size_t array_size = atoi(array_size_string.c_str());

    // parse every array item
    size_t array_item_index = array_size_string.size() + 3;
    while (array_size)
    {
        ResponseType type;
        std::string array_item;
        int header_index = find_header(frame.substr(array_item_index), type);
        assert(header_index == 0);
        size_t response_len = generic_parse(frame.substr(array_item_index), array_item, type);
        // incomplete response
        if (response_len == 0)
        {
            result = "";
            return 0;
        }

        // form response
        result.append(" ").append(array_item);
        array_item_index += array_item.size() + 3;
        array_size--;
    }

    return array_item_index;
}

size_t RedisProtocol::parse_err_string(const std::string &frame, std::string &result)
{
    std::regex match("\\-([^\r\n]+)\r\n");
    std::smatch regex_result;
    if (std::regex_match(frame, regex_result, match))
    {
        result = regex_result[1].str();
        return 3 + result.size();
    }

    return 0;
}

size_t RedisProtocol::parse_number_item(const std::string &frame, std::string &result)
{
    std::regex match(":([0-9]+)\r\n");
    std::smatch regex_result;
    if (std::regex_match(frame, regex_result, match))
    {
        result = regex_result[1].str();
        if (result.front() == '0')
            assert(result.size() == 1);
        return 3 + result.size();
    }

    return 0;
}

size_t RedisProtocol::parse_bulk_string(const std::string &frame, std::string &result)
{
    std::regex match("\\$(\\d+|-1)\r\n([^\r\n]+)\r\n");
    std::smatch regex_result;
    if (std::regex_match(frame, regex_result, match))
    {
        int len = atoi(regex_result[1].str().c_str());
        assert(len != 0);
        if (len == -1)
            result = "nil";
        else
            result = regex_result[2].str();
        return 5 + result.size() + regex_result[1].str().size(); // header+ len(strlen) + 2*CRLF
    }

    return 0;
}

} // namespace reactor