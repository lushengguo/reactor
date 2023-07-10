#pragma once
#ifndef REACTOR_TIMESTAMP_HPP
#define REACTOR_TIMESTAMP_HPP
#include <string>
#include <sys/time.h>
namespace reactor
{
typedef time_t MilliTimestamp;
typedef time_t Timestamp;

// time
std::string fmt_timestamp(time_t t);
std::string readable_current_time();

inline MilliTimestamp get_milli_timestamp()
{
    struct timeval tv;
    ::gettimeofday(&tv, nullptr);
    MilliTimestamp seconds = tv.tv_sec;
    return seconds * 1000 + tv.tv_usec / 1000;
}
} // namespace reactor

#endif