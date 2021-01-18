#include <iostream>
#include <pthread.h>
#include <unistd.h>
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *mywait(void *arg)
{
  pthread_mutex_lock(&mutex);
  pthread_cond_wait(&cond, &mutex);
  pthread_mutex_unlock(&mutex);
  sleep(1);
  std::cout << "wakeup tid=" << pthread_self() << std::endl;
}

int main()
{
  pthread_t tid[10];
  for (int i = 0; i < 10; i++)
  {
    pthread_create(&tid[i], nullptr, mywait, nullptr);
  }
  sleep(1);
  pthread_mutex_lock(&mutex);
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mutex);

  while (true)
  {
  }
}