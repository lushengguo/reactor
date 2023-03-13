#pragma once
#include <regex>
#ifndef REACTOR_MIXED_HPP
#define REACTOR_MIXED_HPP

#include <fcntl.h>
#include <functional>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

#define UNUSED(expr)                                                                                                   \
    do                                                                                                                 \
    {                                                                                                                  \
        (void)expr;                                                                                                    \
    } while (0)

namespace reactor
{
std::string replace_all(std::string_view str, std::string_view from, std::string_view to);

bool is_numeric_string(std::string_view s);

// io
size_t calc_file_size(const char *path);
std::vector<std::string> get_file_names(const char *dir_path);
int read_file(const char *path, std::string &str);
void recursion_create_dir(const char *path);

bool retry_n_times(size_t n, std::function<bool()> func, const char *error_message);
std::string Basename(std::string);

inline bool verify_ipv4(const char *ipv4)
{
    if (not ipv4)
        return false;

    std::regex re("((25[0-5]|(2[0-4]|1\\d|[1-9]|)\\d)\\.?\b){4}");
    return regex_match(ipv4, re);
}

inline bool verify_port(uint16_t port) { return port <= 65535; }
} // namespace reactor

#endif