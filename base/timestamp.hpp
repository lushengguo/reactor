#pragma once
#ifndef REACTOR_TIMESTAMP_HPP
#define REACTOR_TIMESTAMP_HPP
#include <sys/time.h>
namespace reactor
{
typedef time_t mTimestamp;
typedef time_t Timestamp;

inline mTimestamp mtime()
{
    struct timeval tv;
    ::gettimeofday(&tv, nullptr);
    mTimestamp seconds = tv.tv_sec;
    return seconds * 1000 * 1000 + tv.tv_usec;
}
} // namespace reactor

#endif