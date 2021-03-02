#pragma once

#include <time.h>
namespace reactor
{
typedef time_t mTimestamp;
typedef time_t Timestamp;

inline mTimestamp now()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t seconds = tv.tv_sec;
    return seconds * 1000 * 1000 + tv.tv_usec;
}
} // namespace reactor