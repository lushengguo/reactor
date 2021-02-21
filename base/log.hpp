#pragma once

#include "Mutex.h"
#include "timestamp.h"
#include "tools.h"
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
    void           append(std::string header, std::string content, Timestamp t = 0);

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
inline void set_log_directory(const char *path) { Logger::get().set_log_directory(path); }
inline void set_roll_size(size_t size) { Logger::get().set_maxsize_per_file(size); }
inline void set_max_roll_time(size_t size) { Logger::get().set_max_roll(size); }
} // namespace reactor
} // namespace xair

#define log_trace(...)                                                                             \
    xair::iot::Logger::get().append(" TRACE ",                                                     \
      xair::iot::va_list_to_string(__VA_ARGS__) + "  " + xair::iot::Basename(__FILE__) + "-" +     \
        std::to_string(__LINE__))
#define log_debug(...)                                                                             \
    xair::iot::Logger::get().append(" DEBUG ",                                                     \
      xair::iot::va_list_to_string(__VA_ARGS__) + "  " + xair::iot::Basename(__FILE__) + "-" +     \
        std::to_string(__LINE__))
#define log_warn(...)                                                                              \
    xair::iot::Logger::get().append(" WARN  ",                                                     \
      xair::iot::va_list_to_string(__VA_ARGS__) + "  " + xair::iot::Basename(__FILE__) + "-" +     \
        std::to_string(__LINE__))
#define log_info(...)                                                                              \
    xair::iot::Logger::get().append(" INFO  ",                                                     \
      xair::iot::va_list_to_string(__VA_ARGS__) + "  " + xair::iot::Basename(__FILE__) + "-" +     \
        std::to_string(__LINE__))
#define log_error(...)                                                                             \
    xair::iot::Logger::get().append(" ERROR ",                                                     \
      xair::iot::va_list_to_string(__VA_ARGS__) + "  " + xair::iot::Basename(__FILE__) + "-" +     \
        std::to_string(__LINE__))
#define log_fatal(...)                                                                             \
    xair::iot::Logger::get().append(" FATAL ",                                                     \
      xair::iot::va_list_to_string(__VA_ARGS__) + "  " + xair::iot::Basename(__FILE__) + "-" +     \
        std::to_string(__LINE__))

#endif