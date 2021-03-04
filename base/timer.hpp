#pragma once
#ifndef REACTOR_TIMER_HPP
#define REACTOR_TIMER_HPP
#include <string>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

namespace reactor
{
typedef time_t Timestamp;
struct Timer
{
    static std::string readable_time()
    {
        timeval tv;
        gettimeofday(&tv, NULL);
        tv.tv_sec += 8 * 60 * 60;
        struct tm *p;
        p = gmtime(&tv.tv_sec);
        char s[80];
        strftime(s, 80, "%Y-%m-%ld %H:%M:%S.", p);
        std::string st(s + std::to_string(tv.tv_usec));
        return st;
    }
    static time_t nano_time() { return time(NULL); }
};

} // namespace reactor

#endif