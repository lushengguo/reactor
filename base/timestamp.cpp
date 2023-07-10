#include "base/timestamp.hpp"
#include <chrono>
#include <fmt/format.h>
namespace reactor
{
std::string fmt_timestamp(time_t t)
{
    t += 8 * 60 * 60;
    struct tm *p;
    p = gmtime(&t);
    char s[80];
    strftime(s, 80, "%Y-%m-%d %H:%M:%S", p);
    std::string st(s);
    return st;
}

std::string readable_current_time()
{
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto microseconds = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    microseconds %= 1000'000;
    std::time_t time = std::chrono::system_clock::to_time_t(now);
    return fmt_timestamp(time).append(fmt::format(".{:06}", microseconds.count()));
}
} // namespace reactor