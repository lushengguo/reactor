#include "net/Epoller.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpConnection.hpp"
#include <sys/epoll.h>

namespace reactor
{
EventLoop::EventLoop()
  : poller_(new Poller(this)),
    tqueue_(new TimerQueue(this)),
    self_(pthread_self()),
    looping_(false)
{
    pool_.start();
}

void EventLoop::handle_event(mTimestamp receive_time)
{
    assert_in_loop_thread();
    auto events = poller_->active_events();
    for (const epoll_event &event : events)
    {
        // events的size等于当前poller监听描述符的数量
        // poller返回的events会把最后一个活跃的event的下一个event的events设为0
        // 如果存在的话
        if (event.events == 0)
            break;

        //判断是否是定时事件
        if (tqueue_->contain(event.data.fd))
        {
            tqueue_->handle_event(event.data.fd, event.events);
            continue;
        }

        //连接事件
        if (connMap_.count(event.data.fd) == 1)
        {
            connMap_.at(event.data.fd)
              ->handle_event(event.events, receive_time);
        }
    }
}

void EventLoop::loop()
{
    if (looping_)
        return;

    looping_ = true;
    while (true)
    {
        mTimestamp receive_time = poller_->epoll(10);
        run_buffered_task();
        handle_event(receive_time);
    }
}

void EventLoop::update_monitor_object(TcpConnectionPtr conn1)
{
    auto func = [&](TcpConnectionPtr conn) {
        if (connMap_.count(conn->fd()) == 1)
        {
            poller_->modify_monitor_object(conn->fd(), conn->interest_event());
        }
        else
        {
            connMap_.insert(std::make_pair(conn->fd(), conn));
            poller_->new_monitor_object(conn->fd(), conn->interest_event());
        }
    };

    run_in_loop_thread(std::bind(func, conn1));
}

void EventLoop::remove_monitor_object(TcpConnectionPtr conn1)
{
    auto func = [&](TcpConnectionPtr conn) {
        assert(connMap_.count(conn->fd()) == 1);
        poller_->remove_monitor_object(conn->fd());
        // connection的生命周期由EventLoop管理
        // erase后析构自动断开连接(前提是用户没有保存TcpConnectionPtr)
        connMap_.erase(conn->fd());
    };
    run_in_loop_thread(std::bind(func, conn1));
}

void EventLoop::assert_in_loop_thread() const { assert(in_loop_thread()); }

EventLoop::TimerId EventLoop::run_at(
  const EventLoop::TimerTaskCallback &cb, mTimestamp abs_mtime)
{
    return tqueue_->run_at(cb, abs_mtime);
}

EventLoop::TimerId EventLoop::run_after(
  const EventLoop::TimerTaskCallback &cb, mTimestamp after)
{
    return tqueue_->run_after(cb, after);
}

EventLoop::TimerId EventLoop::run_every(
  const EventLoop::TimerTaskCallback &cb, mTimestamp period, mTimestamp after)
{
    return tqueue_->run_every(cb, period, after);
}

void EventLoop::cancel(EventLoop::TimerId id1)
{
    auto func = [&](TimerId id) { tqueue_->cancel(id); };
    run_in_loop_thread(std::bind(func, id1));
}

void EventLoop::new_monitor_object(EventLoop::TimerId id1)
{
    auto func = [&](TimerId id) { poller_->new_monitor_object(id, EPOLLIN); };
    run_in_loop_thread(std::bind(func, id1));
}

void EventLoop::remove_monitor_object(EventLoop::TimerId id1)
{
    auto func = [&](TimerId id) { poller_->remove_monitor_object(id); };
    run_in_loop_thread(std::bind(func, id1));
}

void EventLoop::run_in_loop_thread(const Task &func)
{
    if (in_loop_thread())
    {
        func();
    }
    else
    {
        MutexLockGuard lock(task_queue_mutex_);
        task_queue_.emplace_back(func);
    }
}

void EventLoop::run_in_loop_thread(Task &&func)
{
    if (in_loop_thread())
    {
        func();
    }
    else
    {
        MutexLockGuard lock(task_queue_mutex_);
        task_queue_.emplace_back(std::move(func));
    }
}

bool EventLoop::in_loop_thread() const { return self_ == pthread_self(); }

void EventLoop::run_buffered_task()
{
    assert_in_loop_thread();
    MutexLockGuard lock(task_queue_mutex_);
    while (!task_queue_.empty())
    {
        if (task_queue_.front())
            task_queue_.front()();

        task_queue_.pop_front();
    }
}

void EventLoop::run_in_work_thread(const Task &task)
{
    if (looping_)
    {
        pool_.add_task(task);
    }
    else
    {
        MutexLockGuard lock(task_queue_mutex_);
        task_queue_.emplace_back([&, this]() { run_in_work_thread(task); });
    }
}

void EventLoop::run_in_work_thread(Task &&task)
{
    if (looping_)
    {
        pool_.add_task(std::move(task));
    }
    else
    {
        MutexLockGuard lock(task_queue_mutex_);
        task_queue_.emplace_back(
          [&, this]() { run_in_work_thread(std::move(task)); });
    }
}

} // namespace reactor