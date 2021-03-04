#pragma once
#ifndef REACTOR_LOG_HPP
#define REACTOR_LOG_HPP

#include "base/mixed.hpp"
#include "base/mutex.hpp"
#include "base/timestamp.hpp"
#include <map>
#include <stdarg.h>
#include <string.h>
#include <string>
#include <vector>

namespace reactor
{
class Logger
{
  public:
    typedef std::string                Filename;
    typedef std::string                Filepath;
    typedef std::vector<Filename>      Filenames;
    typedef std::map<time_t, Filepath> FileMap;

    static Logger &get();
    void           set_log_directory(const char *dir);
    void           set_maxsize_per_file(size_t size) { roll_size_ = size; }
    void           set_max_roll(size_t roll);
    void           set_print_option(bool en) { enable_print_ = en; }
    void append(std::string header, std::string content, Timestamp t = 0);

  private:
    Logger();
    ~Logger();

    void    print(std::string) const;
    void    save(std::string);
    void    check_roll();
    FileMap rolling_files() const;
    void    remove_roll_out_files() const;

    std::string filename_;
    std::string basename_;
    std::string dirname_;
    std::string strip_path_;
    size_t      roll_size_;
    size_t      max_roll_;
    int         fd_;
    bool        enable_print_;
    Mutex       mutex_;
};

inline std::string va_list_to_string(const char *format, ...)
{
    char    buf[4096];
    va_list list;
    va_start(list, format);
    vsnprintf(buf, 4096, format, list);
    va_end(list);
    return std::string(buf);
}

inline void disable_log_print() { Logger::get().set_print_option(false); }
inline void set_log_directory(const char *path)
{
    Logger::get().set_log_directory(path);
}
inline void set_roll_size(size_t size)
{
    Logger::get().set_maxsize_per_file(size);
}
inline void set_max_roll_time(size_t size) { Logger::get().set_max_roll(size); }
} // namespace reactor

#define log_trace(...)                                                         \
    reactor::Logger::get().append(" TRACE ",                                   \
      reactor::va_list_to_string(__VA_ARGS__)                                  \
        .append(" ")                                                           \
        .append(reactor::Basename(__FILE__))                                   \
        .append("-")                                                           \
        .append(std::to_string(__LINE__)))
#define log_debug(...)                                                         \
    reactor::Logger::get().append(" DEBUG ",                                   \
      reactor::va_list_to_string(__VA_ARGS__)                                  \
        .append(" ")                                                           \
        .append(reactor::Basename(__FILE__))                                   \
        .append("-")                                                           \
        .append(std::to_string(__LINE__)))
#define log_warn(...)                                                          \
    reactor::Logger::get().append(" WARN  ",                                   \
      reactor::va_list_to_string(__VA_ARGS__)                                  \
        .append(" ")                                                           \
        .append(reactor::Basename(__FILE__))                                   \
        .append("-")                                                           \
        .append(std::to_string(__LINE__)))
#define log_info(...)                                                          \
    reactor::Logger::get().append(" INFO  ",                                   \
      reactor::va_list_to_string(__VA_ARGS__)                                  \
        .append(" ")                                                           \
        .append(reactor::Basename(__FILE__))                                   \
        .append("-")                                                           \
        .append(std::to_string(__LINE__)))
#define log_error(...)                                                         \
    reactor::Logger::get().append(" ERROR ",                                   \
      reactor::va_list_to_string(__VA_ARGS__)                                  \
        .append(" ")                                                           \
        .append(reactor::Basename(__FILE__))                                   \
        .append("-")                                                           \
        .append(std::to_string(__LINE__)))
#define log_fatal(...)                                                         \
    reactor::Logger::get().append(" FATAL ",                                   \
      reactor::va_list_to_string(__VA_ARGS__)                                  \
        .append(" ")                                                           \
        .append(reactor::Basename(__FILE__))                                   \
        .append("-")                                                           \
        .append(std::to_string(__LINE__)))
#endif