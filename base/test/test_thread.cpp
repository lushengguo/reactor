#include "base/log.hpp"
#include "base/thread.hpp"
#include <unistd.h>
void func(const char *str, int repeat)
{
  while (repeat--)
  {
    if (str)
    {
      LOG_TRACE("%s", str);
      LOG_TRACE("current tid=%ld", pthread_self());
    }
    sleep(1);
  }
}

int main()
{
  reactor::Thread thread(std::bind(func, "test_thread", 10));
  thread.start();
  LOG_TRACE("main thread tid=%ld", pthread_self());
  thread.join();
}