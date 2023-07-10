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
    Mutex()
    {
        int r = pthread_mutex_init(&mutex_, 0);
        assert(r == 0);
    }

    ~Mutex()
    {
        int r = pthread_mutex_destroy(&mutex_);
        assert(r == 0);
    }

    void lock()
    {
        int r = pthread_mutex_lock(&mutex_);
        assert(r == 0);
    }

    void unlock()
    {
        int r = pthread_mutex_unlock(&mutex_);
        assert(r == 0);
    }

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