
#include "log.hpp"
#include <pthread.h>

void *create_log(void *arg)
{
  int i = 10000;
  while (i--)
  {
    if (i % 3 == 0)
      LOG_WARN("log----");
    else if (i % 5 == 0)
      LOG_ERROR("log----");
    else if (i % 7 == 0)
      LOG_TRACE("log----");
  }
}

int main()
{
  pthread_t tid;
  pthread_create(&tid, NULL, create_log, NULL);
  pthread_detach(tid);
  create_log(NULL);
}