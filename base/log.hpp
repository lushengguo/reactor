#pragma once
#include "base/mutex.hpp"
#include "base/noncopyable.hpp"
#include "base/timer.hpp"
#include <fstream>
#include <map>
#include <stdarg.h>
#include <string.h>
#include <string>

namespace reactor
{
// compile time calculate basename
struct Filename
{
  Filename(std::string name) : filename_(name){};
  std::string str()
  {
    char buf[filename_.size() + 1];
    strcpy(buf, filename_.c_str());
    return basename(buf);
  };

private:
  std::string filename_;
};

class Logger : public noncopyable
{
public:
  enum LoggerLevel
  {
    kTrace,
    kDebug,
    kInfo,
    kWarn,
    kError,
    kFatal
  };
  static Logger &unique_logger();
  void add(LoggerLevel level, const std::string &content, reactor::Filename filename,
           int line);
  bool set_storge_dir(std::string dirpath);
  bool set_max_roll(size_t max_roll);
  bool set_max_file_size(size_t size);

private:
  Logger(size_t max_roll = 5, size_t max_file_size = 2 * 1024 * 1024,
         std::string dirpath = "./log")
    : max_roll_(max_roll), max_file_size_(max_file_size), mutex_(new Mutex()),
      dirpath_(dirpath)
  {
    size_     = 0;
    basename_ = "log.txt";
  }

private:
  typedef std::map<Timestamp, std::string> FileMap;

  // append到文件里，顺带提供文件大小计算功能
  void        save(const std::string &content);
  size_t      curr_size() const { return size_; }
  std::string log_level_to_string(LoggerLevel);

  //重新打开log文件
  void        reopen_fstream();
  std::string filepath() const { return dirpath_ + "/" + basename_; }

  // roll
  void    checkroll();
  void    remove_rollout() const;
  FileMap roll_files() const;

  //当前打开的日志文件大小
  size_t size_;

  // int         fd_;
  std::ofstream ofstream_;
  size_t        max_roll_;
  size_t        max_file_size_;
  std::string   basename_;
  Mutex *       mutex_;
  std::string   dirpath_;

  static Logger *logger;
};

inline std::string var_list_to_string(const char *format, ...)
{
  char    buf[4096];
  va_list list;
  va_start(list, format);
  vsnprintf(buf, 4096, format, list);
  va_end(list);
  return std::string(buf);
}

#define LOG_TRACE(...)                                                                   \
  reactor::Logger::unique_logger().add(reactor::Logger::kTrace,                          \
                                       reactor::var_list_to_string(__VA_ARGS__),         \
                                       reactor::Filename(__FILE__), __LINE__)
#define LOG_DEBUG(...)                                                                   \
  reactor::Logger::unique_logger().add(reactor::Logger::kDebug,                          \
                                       reactor::var_list_to_string(__VA_ARGS__),         \
                                       reactor::Filename(__FILE__), __LINE__)
#define LOG_INFO(...)                                                                    \
  reactor::Logger::unique_logger().add(reactor::Logger::kInfo,                           \
                                       reactor::var_list_to_string(__VA_ARGS__),         \
                                       reactor::Filename(__FILE__), __LINE__)
#define LOG_WARN(...)                                                                    \
  reactor::Logger::unique_logger().add(reactor::Logger::kWarn,                           \
                                       reactor::var_list_to_string(__VA_ARGS__),         \
                                       reactor::Filename(__FILE__), __LINE__)
#define LOG_ERROR(...)                                                                   \
  reactor::Logger::unique_logger().add(reactor::Logger::kError,                          \
                                       reactor::var_list_to_string(__VA_ARGS__),         \
                                       reactor::Filename(__FILE__), __LINE__)
#define LOG_FATAL(...)                                                                   \
  reactor::Logger::unique_logger().add(reactor::Logger::kFatal,                          \
                                       reactor::var_list_to_string(__VA_ARGS__),         \
                                       reactor::Filename(__FILE__), __LINE__)
} // namespace reactor