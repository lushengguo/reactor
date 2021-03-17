#pragma once
#ifndef REACTOR_MIXED_HPP
#define REACTOR_MIXED_HPP

#include <fcntl.h>
#include <functional>
#include <string>
#include <string_view>
#include <unistd.h>
#include <vector>

#define UNUSED(expr)                                                           \
    do                                                                         \
    {                                                                          \
        (void)expr;                                                            \
    } while (0)

namespace reactor
{
std::string replace_all(
  std::string_view str, std::string_view from, std::string_view to);

bool is_numeric_string(std::string_view s);

// io
size_t                   calc_file_size(const char *path);
std::vector<std::string> get_file_names(const char *dir_path);
int                      read_file(const char *path, std::string &str);
void                     recursion_create_dir(const char *path);

bool retry_n_times(
  size_t n, std::function<bool()> func, const char *error_message);
std::string Basename(std::string);
} // namespace reactor

#endif