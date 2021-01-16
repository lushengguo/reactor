#pragma once
#include <functional>
#include <pthread.h>
namespace reactor
{
class Thread
{
public:
  typedef std::function<void(void)> Func;

  void run(Func fun);
};
} // namespace reactor