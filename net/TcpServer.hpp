#pragma once
#ifndef REACTOR_TCPSERVER_HPP
#define REACTOR_TCPSERVER_HPP

#include "base/noncopyable.hpp"
#include "base/threadpool.hpp"
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
    using MessageFunc = typename TcpConnection::MessageFunc;
    typedef std::function<void()> EventCallback;

    TcpServer(EventLoop *loop, INetAddr &&addr, std::string_view server_name)
      : endpoint_(std::move(addr)),
        name_(server_name),
        started_(false),
        ploop_(loop)
    {}

    ~TcpServer();

    void start();

    void set_onConnectionCallback(const EventCallback &cb)
    {
        onConnectionCallback_ = cb;
    }

    void set_onMessageCallback(const MessageFunc &cb)
    {
        onMessageCallback_ = cb;
    }

  private:
    void listen();
    void accept();

    TcpConnectionPtr self_;

    INetAddr    endpoint_;
    std::string name_;
    bool        started_;
    EventLoop * ploop_;
    Socket      sock_;

    MessageFunc   onMessageCallback_;
    EventCallback onConnectionCallback_;
    EventCallback onWriteCompleteCallback_;
};
} // namespace reactor

#endif