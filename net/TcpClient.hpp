#pragma once
#ifndef REACTOR_TCPCLIENT_HPP
#define REACTOR_TCPCLIENT_HPP

#include "base/Buffer.hpp"
#include "net/INetAddr.hpp"
#include "net/Socket.hpp"
#include "net/TcpConnection.hpp"
#include <string_view>
namespace reactor
{
class EventLoop;
class TcpClient : private noncopyable
{
  public:
    TcpClient(
      EventLoop *loop, const INetAddr &server_addr, std::string_view name)
      : loop_(loop), server_endpoint_(server_addr), started_(false), name_(name)
    {}

    using MessageFunc           = typename TcpConnection::MessageFunc;
    using EventCallback         = typename TcpConnection::EventCallback;
    using NonInputParamCallback = typename TcpConnection::NonInputParamCallback;

    void start();

    void set_onConnectionCallback(const EventCallback &cb)
    {
        onConnectionCallback_ = cb;
    }

    void set_onConnectionCallback(EventCallback &&cb)
    {
        onConnectionCallback_ = std::move(cb);
    }

    void set_onMessageCallback(const MessageFunc &cb)
    {
        onMessageCallback_ = cb;
    }

    void set_onMessageCallback(MessageFunc &&cb)
    {
        onMessageCallback_ = std::move(cb);
    }

    void set_onServerCloseCallback(const NonInputParamCallback &cb)
    {
        onServerCloseCallback_ = cb;
    }

    void set_onServerCloseCallback(NonInputParamCallback &&cb)
    {
        onServerCloseCallback_ = std::move(cb);
    }

    // default
    void onServerCloseCallback();

  private:
    TcpConnectionPtr self_connection_;

    EventLoop * loop_;
    INetAddr    server_endpoint_;
    bool        started_;
    Socket      sock_;
    std::string name_;

    MessageFunc           onMessageCallback_;
    EventCallback         onConnectionCallback_;
    EventCallback         onWriteCompleteCallback_;
    NonInputParamCallback onServerCloseCallback_;
};
} // namespace reactor

#endif