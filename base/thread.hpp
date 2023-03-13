#pragma once
#ifndef REACTOR_THREAD_HPP
#define REACTOR_THREAD_HPP
#include "base/log.hpp"
#include <functional>
#include <pthread.h>
namespace reactor
{
class ThreadFuncCaller;
//抽象了线程的操作
//线程类只是一个载体，用户决定它要做什么
class Thread
{
  public:
    typedef std::function<void(void)> Func;

    Thread(const Func &func) : func_(func), started_(false), tid_(-1) {}

    void detach();
    Thread &start();
    void join();
    bool started() const { return started_; }

  protected:
    //避免用户调用 做了一层转发
    friend class ThreadFuncCaller;
    void run_func();

  private:
    Func func_;
    bool started_;
    pthread_t tid_;
};
} // namespace reactor

#endif