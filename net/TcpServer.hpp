#pragma once

#include "base/ThreadPool.hpp"
#include "base/noncopyable.hpp"
#include "net/Acceptor.hpp"
#include "net/EventLoop.hpp"
#include "net/INetAddr.hpp"
#include "net/Socket.hpp"
#include "net/TcpConnection.hpp"
#include <functional>
#include <memory>

namespace reactor
{
class TcpServer : private noncopyable
{
  public:
    typedef std::function<void()> EventCallback;
    TcpServer(EventLoop *loop, const INetAddr &addr);

    int  set_thread_num(size_t n);
    void start();
    void wait_connection();
    void new_connection(int fd);
    void refuse_connection() const;

    void set_onConnectionCallback(const EventCallback &cb)
    {
        onConnectionCallback_ = cb;
    }

    void set_onMessageCallback(const EventCallback &cb)
    {
        onMessageCallback_ = cb;
    }

  private:
    Socket * sock_;
    INetAddr addr_;

    std::unique_ptr<ThreadPool> ThreadPool_;
    std::shared_ptr<EventLoop>  loop_;

    EventCallback onMessageCallback_;
    EventCallback onConnectionCallback_;
    EventCallback onWriteCompleteCallback_;
};
} // namespace reactor