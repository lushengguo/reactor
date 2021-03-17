#include "base/log.hpp"
#include "base/threadpool.hpp"
#include <unistd.h>
void func(const char *str, int repeat)
{
    while (repeat--)
    {
        if (str)
        {
            log_trace("%s", str);
            log_trace("current tid=%ld", pthread_self());
        }
        sleep(1);
    }
}

int main()
{
    reactor::ThreadPool thread_poll;
    thread_poll.start();
    int task_num = 25;
    log_trace("main thread tid=%ld", pthread_self());
    while (task_num--) thread_poll.add_task(std::bind(func, "test_thread", 10));
    while (true) {}
}