#include "base/log.hpp"
#include "base/thread.hpp"
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
    reactor::Thread thread(std::bind(func, "test_thread", 10));
    thread.start();
    log_trace("main thread tid=%ld", pthread_self());
    thread.join();
}