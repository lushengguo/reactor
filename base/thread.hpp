#pragma once
#ifndef REACTOR_THREAD_HPP
#define REACTOR_THREAD_HPP
#include <functional>
#include <pthread.h>
namespace reactor
{
//抽象了线程的操作
//线程类只是一个载体，用户决定它要做什么
class Thread
{
  public:
    typedef std::function<void(void)> Func;

    Thread(const Func &func) : func_(func), start_(false), tid_(-1) {}

    void start();
    void join();
    bool started() const { return start_; }

    //用户不要调用啊！！！
    void run_func();

  private:
    Func      func_;
    bool      start_;
    pthread_t tid_;
};
} // namespace reactor

#endif