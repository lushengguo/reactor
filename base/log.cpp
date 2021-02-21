#include "log.h"

#include "tools.h"
#include <algorithm>
#include <fcntl.h>
#include <fstream>
#include <functional>
#include <iostream>
#include <libgen.h>
#include <map>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
#define KB (1024)
#define MB (1024 * KB)
namespace reactor
{
Logger::Logger()
{
    fd_           = -1;
    enable_print_ = true;
    max_roll_     = 10;
    roll_size_    = 100 * KB;
    dirname_      = "./log";
    basename_     = "log.txt";
    filename_     = dirname_ + "/" + basename_;
    set_log_directory(dirname_.c_str());
    strip_path_ = dirname_ + "/striplog";
}

Logger &Logger::get()
{
    static Logger logger;
    return logger;
}

Logger::~Logger()
{
    if (fd_ != -1)
        close(fd_);
}

void Logger::set_max_roll(size_t roll) { max_roll_ = roll; }

void Logger::set_log_directory(const char *dir)
{
    if (!dir && dir[0] != '\0')
        return;

    dirname_  = dir;
    filename_ = dirname_ + "/" + basename_;
    recursion_create_dir(dirname_.c_str());

    int fd = open(filename_.c_str(), O_CREAT | O_WRONLY | O_CLOEXEC | O_APPEND, DEFFILEMODE);
    if (fd < 0)
    {
        perror("open log file failed");
        return;
    }

    if (fd_ == -1)
    {
        fd_ = fd;
    }
    else
    {
        dup2(fd, fd_);
    }
}

void Logger::append(std::string header, std::string content, Timestamp t)
{
    MutexLockGuard lock(mutex_);
    std::string    now;
    t == 0 ? now = fmt_timestamp(time(nullptr)) : now = fmt_timestamp(t);
    std::string s(now + +" |" + header + "| " + content + "\n");
    print(s);
    // Trace不保存
    if (strcasestr(header.c_str(), "trace") == nullptr)
    {
        save(s);
    }
}

void Logger::print(std::string content) const
{
    if (enable_print_)
    {
        std::cout << content << std::flush;
    }
}

void Logger::save(std::string content)
{
    if (fd_ == -1)
        return;

    write(fd_, content.c_str(), content.size());
    check_roll();
}

void Logger::check_roll()
{
    time_t now       = time(nullptr);
    size_t file_size = calc_file_size(filename_.c_str());
    tcflush(fd_, TCOFLUSH);

    if (file_size > roll_size_ && max_roll_ > 0)
    {
        // log.txt文件改名，后面加上下划线和时间戳，新建个log.txt存日志
        std::string newname = filename_ + "_" + std::to_string(now);
        if (rename(filename_.c_str(), newname.c_str()) != 0)
        {
            fprintf(stderr,
              "rename logfile %s -> %s failed ; msg: %s\n",
              filename_.c_str(),
              newname.c_str(),
              strerror(errno));
            return;
        }

        //重新指定fd_指向的文件
        set_log_directory(dirname_.c_str());

        //检查本地log文件是否过多，多的话删除至最多max_roll_time_个
        remove_roll_out_files();
    }
}

void Logger::remove_roll_out_files() const
{
    //删除多出的日志文件
    FileMap rfiles = rolling_files();
    if (rfiles.size() < max_roll_)
        return;

    // map对时间戳排序，越早的日志文件排在越前面
    for (FileMap::const_iterator iter = rfiles.begin(); iter != rfiles.end(); ++iter)
    {
        if (remove(iter->second.c_str()) != 0)
            std::cerr << "remove file error : " << iter->second << std::endl;
        else
            rfiles.erase(iter);

        if (rfiles.size() == max_roll_)
            break;
    }
}

//统计log目录下所有滚动文件
//返回值 键-滚动时间戳 值-文件绝对路径
Logger::FileMap Logger::rolling_files() const
{
    FileMap   rfiles;
    Filenames filenames = get_file_names(dirname_.c_str());
    for (Filename &name : filenames)
    {
        size_t index = name.find(basename_ + "_");
        if (index != std::string::npos)
        {
            time_t t = atoi(name.substr(basename_.size() + 1).c_str());
            rfiles.insert(std::pair<time_t, Filepath>(t, dirname_ + "/" + name));
        }
    }

    return rfiles;
}

} // namespace reactor