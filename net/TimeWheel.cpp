#include "TimeWheel.hpp"
#include <limits>

namespace reactor
{
TimeWheel::TimeWheel() : timeout_(std::numeric_limits<size_t>::max()) {}

void TimeWheel::insert(FileDescripter fd)
{
    time_t now = ::time(nullptr);
    if (fd_locater_.contains(fd))
    {
        auto iter = fd_locater_.at(fd);
        if (iter->first == now)
            return;

        iter->second.erase(fd);
        fd_locater_.erase(fd);
        if (iter->second.empty())
            time_wheel_.erase(iter);
    }

    if (time_wheel_.empty() || time_wheel_.back().first != now)
    {
        time_wheel_.emplace_back(now, std::set<FileDescripter>{fd});
    }
    else
    {
        time_wheel_.back().second.insert(fd);
    }

    fd_locater_[fd] = time_wheel_.rbegin().base();
}

void TimeWheel::erase(FileDescripter fd)
{
    if (not fd_locater_.contains(fd))
        return;

    auto iter = fd_locater_.at(fd);
    iter->second.erase(fd);
    fd_locater_.erase(fd);
    if (iter->second.empty())
        time_wheel_.erase(iter);
}

std::set<TimeWheel::FileDescripter> TimeWheel::update()
{
    std::set<TimeWheel::FileDescripter> r;
    time_t now = time(nullptr);
    time_t timeout = now - timeout_;
    std::erase_if(time_wheel_, [&](auto &&node) {
        if (node.first < timeout)
            for (auto fd : node.second)
                r.insert(fd);

        return node.first < timeout;
    });
    return r;
}

void TimeWheel::set_timeout(time_t t) { timeout_ = t; }
} // namespace reactor