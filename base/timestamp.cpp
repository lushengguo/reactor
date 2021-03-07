#include "base/timestamp.hpp"

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

time_t reverse_fmt_timestamp(const char *tstring)
{
    if (!tstring || tstring[0] == '\0')
        return 0;

    struct tm *tmp_time = (struct tm *)malloc(sizeof(struct tm));
    strptime(tstring, "%Y-%m-%d %H:%M:%S", tmp_time);
    time_t t = mktime(tmp_time);
    free(tmp_time);
    return t - 16 * 60 * 60;
}

std::string readable_current_time() { return fmt_timestamp(time(nullptr)); }
} // namespace reactor