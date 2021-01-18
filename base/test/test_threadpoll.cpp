#include "base/log.hpp"
#include "base/threadpoll.hpp"
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
  reactor::ThreadPoll thread_poll;
  thread_poll.start();
  int task_num = 25;
  LOG_TRACE("main thread tid=%ld", pthread_self());
  while (task_num--) thread_poll.add_task(std::bind(func, "test_thread", 10));
  while (true)
  {
  }
}