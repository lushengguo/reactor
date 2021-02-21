
#include "base/log.hpp"
#include <pthread.h>

void *create_log(void *arg)
{
    int i = 1000000;
    while (i--)
    {
        if (i % 3 == 0)
            log_warn("log----");
        else if (i % 5 == 0)
            log_error("log----");
        else if (i % 7 == 0)
            log_trace("log----");
    }

    return NULL;
}

int main()
{
    pthread_t tid;
    pthread_create(&tid, NULL, create_log, NULL);
    pthread_detach(tid);
    create_log(NULL);
}