#pragma once
#ifndef REDIS_PROTOCOL_PARSER_HPP
#define REDIS_PROTOCOL_PARSER_HPP

#include <memory>
#include <string>
#include <vector>

namespace reactor
{

// response protocol parser
// response format: https://redis.io/topics/protocol

// For Simple Strings the first byte of the reply is "+"
// For Arrays the first byte of the reply is "*"
// For Errors the first byte of the reply is "-"
// For Integers the first byte of the reply is ":"
// For Bulk Strings the first byte of the reply is "$"
class RedisProtocol
{
  public:
    enum ResponseType
    {
        kSimpleStringItem,
        kArrayItem,
        kErrString,
        kNumberItem,
        kBulkString
    };
    // reutrn 0 if parse failed ,else return should retrive length
    size_t parse_response(const std::string &frame, std::string &result);

  private:
    // return -1 if find nothing else return index of header
    int find_header(const std::string &frame, ResponseType &type);

    size_t generic_parse(
      const std::string &frame, std::string &result, ResponseType type);

    size_t parse_simple_string_item(
      const std::string &frame, std::string &result);
    size_t parse_array_item(const std::string &frame, std::string &result);
    size_t parse_err_string(const std::string &frame, std::string &result);
    size_t parse_number_item(const std::string &frame, std::string &result);
    size_t parse_bulk_string(const std::string &frame, std::string &result);
};

} // namespace reactor
#endif /* REDIS_PROTOCOL_PARSER_HPP */
