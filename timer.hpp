#pragma once
#include <string>
#include <unistd.h>
namespace reactor
{
typedef time_t Timestamp;
struct Timer
{
  static std::string readable_time();
  static time_t      nano_time();
};

} // namespace reactor