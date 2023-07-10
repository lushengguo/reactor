#pragma once
#ifndef REACTOR_TIMEWHEEL_HPP
#define REACTOR_TIMEWHEEL_HPP

#include <list>
#include <set>
#include <time.h>
#include <unordered_map>

namespace reactor
{
class TimeWheel
{
  public:
    typedef int FileDescripter;

  public:
    TimeWheel();

    void insert(FileDescripter fd);
    void erase(FileDescripter fd);

    std::set<FileDescripter> update();
    void set_timeout(time_t);

  private:
    typedef std::list<std::pair<time_t, std::set<FileDescripter>>> Container;

    Container time_wheel_;
    std::unordered_map<FileDescripter, Container::iterator> fd_locater_;
    time_t timeout_;
};
} // namespace reactor

#endif