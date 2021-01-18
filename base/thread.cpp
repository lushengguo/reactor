#include "base/thread.hpp"
#include <assert.h>
using namespace reactor;

namespace launch
{
static void *run(void *object)
{
  Thread *thread = reinterpret_cast<reactor::Thread *>(object);
  thread->run_func();
  return NULL;
}
} // namespace launch

void Thread::start()
{
  if (!start_)
    assert(pthread_create(&tid_, NULL, launch::run, this) == 0);
}

void Thread::join()
{
  pthread_join(tid_, NULL);
  start_ = false;
}

void Thread::run_func()
{
  if (func_)
  {
    start_ = true;
    func_();
  }
}