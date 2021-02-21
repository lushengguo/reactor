#pragma once
#include "base/noncopyable.hpp"
#include "net/Channel.hpp"

namespace reactor
{
class Poller : private noncopyable
{
  public:
    bool add_channel(Channel *ch);
    bool has_channel(const Channel &ch);
    bool remove_channel(const Channel &ch);
};

} // namespace reactor