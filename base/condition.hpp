#pragma once
#ifndef REACTOR_CONDITION_HPP
#define REACTOR_CONDITION_HPP

#include "base/mutex.hpp"
#include <assert.h>
#include <pthread.h>
namespace reactor
{
class Condition
{
  public:
    Condition()
    {
        int r = pthread_cond_init(&cond_, nullptr);
        assert(r == 0);
    }

    ~Condition()
    {
        int r = pthread_cond_destroy(&cond_);
        assert(r == 0);
    }

    void broadcast()
    {
        mutex_.lock();
        assert(pthread_cond_broadcast(&cond_) == 0);
        mutex_.unlock();
    }

    void wait()
    {
        mutex_.lock();
        assert(pthread_cond_wait(&cond_, mutex_.get()) == 0);
        mutex_.unlock();
    }

    void signal() { assert(pthread_cond_signal(&cond_) == 0); }

  private:
    Mutex mutex_;
    pthread_cond_t cond_;
};
} // namespace reactor
#endif