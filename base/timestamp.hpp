#pragma once
#ifndef REACTOR_TIMESTAMP_HPP
#define REACTOR_TIMESTAMP_HPP
#include <string>
#include <sys/time.h>
namespace reactor
{
typedef time_t mTimestamp;
typedef time_t Timestamp;

// time
std::string fmt_timestamp(time_t t);
time_t      reverse_fmt_timestamp(const char *tstring);
std::string readable_current_time();

inline mTimestamp mtime()
{
    struct timeval tv;
    ::gettimeofday(&tv, nullptr);
    mTimestamp seconds = tv.tv_sec;
    return seconds * 1000 * 1000 + tv.tv_usec;
}
} // namespace reactor

#endif