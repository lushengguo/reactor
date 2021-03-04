#pragma once
#include <assert.h>
#include <pthread.h>

namespace reactor
{
class Condition;
// auto initialized
// use this for avoiding memory problem
class Mutex
{
  public:
    Mutex() { assert(pthread_mutex_init(&mutex_, 0) == 0); }
    ~Mutex() { assert(pthread_mutex_destroy(&mutex_) == 0); }

    void lock() { assert(pthread_mutex_lock(&mutex_) == 0); }
    void unlock() { assert(pthread_mutex_unlock(&mutex_) == 0); }

  protected:
    friend class Condition;
    pthread_mutex_t *get() { return &mutex_; };

  private:
    pthread_mutex_t mutex_;
};

// RAII auto unlock
class MutexLockGuard
{
  public:
    MutexLockGuard(Mutex &mutex) : mutex_(mutex) { mutex_.lock(); }
    ~MutexLockGuard() { mutex_.unlock(); }

  private:
    Mutex mutex_;
};
} // namespace reactor