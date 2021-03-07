#include "base/thread.hpp"
#include <assert.h>

namespace reactor
{
class ThreadFuncCaller
{
  public:
    void call(Thread *thread) { thread->run_func(); }
};
} // namespace reactor

namespace launch
{
static reactor::ThreadFuncCaller caller;

static void *run(void *object)
{
    reactor::Thread *thread = reinterpret_cast<reactor::Thread *>(object);
    caller.call(thread);
    return nullptr;
}
} // namespace launch

namespace reactor
{

void Thread::start()
{
    if (started_)
        return;

    started_ = true;
    assert(pthread_create(&tid_, nullptr, launch::run, this) == 0);
}

void Thread::join()
{
    pthread_join(tid_, nullptr);
    started_ = false;
}

void Thread::run_func()
{
    if (func_)
        func_();
}
} // namespace reactor