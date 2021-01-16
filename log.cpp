#include "log.hpp"
#include <dirent.h>
#include <iostream>
#include <string.h>
#include <vector>

using namespace reactor;

Logger *Logger::logger = nullptr;

Logger &Logger::unique_logger()
{
  if (!logger)
    logger = new Logger();
  return *logger;
}

void Logger::reopen_fstream()
{
  std::string mkparent_cmd = "mkdir -p " + dirpath_;
  system(mkparent_cmd.c_str());
  assert(!ofstream_.is_open());
  ofstream_.open(dirpath_ + "/" + basename_);
  assert(ofstream_.is_open());
}

std::string Logger::log_level_to_string(LoggerLevel level)
{
  switch (level)
  {
  case kTrace:
    return "[trace]";
  case kDebug:
    return "[debug]";
  case kInfo:
    return "[info ]";
  case kWarn:
    return "[warn ]";
  case kError:
    return "[errro]";
  case kFatal:
    return "[fatal]";
  default:
    return "[invalid level]";
  }
}

void Logger::save(const std::string &content)
{
  assert(ofstream_.is_open());
  ofstream_ << content;
  size_ += content.size();
}

void Logger::add(LoggerLevel level, const std::string &content, Filename filename,
                 int line)
{
  MutexLockGuard lock(mutex_);
  save(Timer::readable_time() + log_level_to_string(level) + content + " " +
       filename.str() + "-" + std::to_string(line));
  checkroll();
}

void Logger::checkroll()
{
  if (curr_size() >= max_file_size_)
  {
    ofstream_.close();

    // roll时将log.txt改为log.txt_${timestamp}
    std::string newpath = filepath() + "_" + std::to_string(time(NULL));
    if (rename(filepath().c_str(), newpath.c_str()) != 0)
    {
      fprintf(stderr, "rename logfile failed", strerror(errno));
      return;
    }

    reopen_fstream();
    remove_rollout();
  }
}

void Logger::remove_rollout() const
{
  FileMap fmp = roll_files();
  if (fmp.size() < max_roll_)
    return;

  // map对时间戳排序，越早的日志文件排在越前面
  for (FileMap::const_iterator iter = fmp.begin(); iter != fmp.end(); ++iter)
  {
    if (remove(iter->second.c_str()) != 0)
      std::cerr << "remove file failed ? " << iter->second << std::endl;
    else
      fmp.erase(iter);

    if (fmp.size() == max_roll_)
      break;
  }
}

Logger::FileMap Logger::roll_files() const
{
  auto get_files_under_dir = [](std::string dirpath) -> std::vector<std::string> {
    std::vector<std::string> files;
    DIR *                    dir;
    struct dirent *          ent;
    if ((dir = opendir(dirpath.c_str())) != NULL)
    {
      while ((ent = readdir(dir)) != NULL)
      {
        files.push_back(ent->d_name);
      }
      closedir(dir);
    }

    return files;
  };

  typedef std::string      FileFullPath;
  FileMap                  fmp;
  std::vector<std::string> filenames = get_files_under_dir(dirpath_);
  for (std::string &name : filenames)
  {
    size_t index = name.find(basename_ + "_");
    if (index != std::string::npos)
    {
      time_t timestamp = atoi(name.substr(basename_.size() + 1).c_str());
      fmp.insert(std::pair<time_t, FileFullPath>(timestamp, dirpath_ + "/" + name));
    }
  }

  return fmp;
}

bool Logger::set_storge_dir(std::string dirpath)
{
  dirpath_ = dirpath;

  while (!dirpath_.empty() && dirpath_.rfind('/') == dirpath_.size() - 1)
  {
    dirpath_.pop_back();
  }

  if (dirpath.empty())
    dirpath = ".";

  reopen_fstream();
  return true;
}

bool Logger::set_max_roll(size_t max_roll) { max_roll_ = max_roll; }

bool Logger::set_max_file_size(size_t size)
{
  if (size != 0)
  {
    max_file_size_ = size;
    return true;
  }
  return false;
}
