#pragma once
#include "mutex.hpp"
#include <assert.h>
#include <pthread.h>
namespace reactor
{
class Condition
{
public:
  Condition() : mutex_(new Mutex()) { assert(pthread_cond_init(&cond_) == 0); }
  ~Condition()
  {
    delete mutex_;
    assert(pthread_cond_destroy(&cond_) == 0);
  }

  void broadcast() { assert(pthread_cond_broadcast(&cond_) == 0); }
  void wait() { assert(pthread_cond_wait(&cond, mutex_->get()) == 0); }
  void signal() { assert(pthread_cond_signal(&cond_) == 0); }

private:
  Mutex *        mutex_;
  pthread_cond_t cond_;
};
} // namespace reactor