#pragma once

#include "base/ThreadPool.hpp"
#include "base/noncopyable.hpp"
#include "net/EventLoop.hpp"
#include "net/INetAddr.hpp"
#include "net/Socket.hpp"
#include <functional>
#include <memory>
#include <string>
#include <string_view>

namespace reactor
{
class TcpServer : private noncopyable
{
  public:
    typedef std::function<void()> EventCallback;
    TcpServer(
      EventLoop *loop, const INetAddr &addr, std::string_view server_name)
      : endpoint_(addr), name_(server_name), started_(false), ploop_(loop)
    {}

    ~TcpServer();

    void start();

    void set_onConnectionCallback(const EventCallback &cb)
    {
        onConnectionCallback_ = cb;
    }

    void set_onMessageCallback(const EventCallback &cb)
    {
        onMessageCallback_ = cb;
    }

  private:
    void listen();
    void accept();

    TcpConnectionPtr self_;

    INetAddr      endpoint_;
    std::string   name_;
    bool          started_;
    EventLoop *   ploop_;
    Socket        sock_;
    EventCallback onMessageCallback_;
    EventCallback onConnectionCallback_;
    EventCallback onWriteCompleteCallback_;
};
} // namespace reactor