#pragma once
#ifndef REACTOR_TCPCONNECTION_HPP
#define REACTOR_TCPCONNECTION_HPP

#include "base/mutex.hpp"
#include "base/noncopyable.hpp"
#include "base/timestamp.hpp"
#include "net/Socket.hpp"
#include <functional>
#include <memory>
#include <string_view>

namespace reactor
{

class Buffer;
class EventLoop;
class TcpConnection;
typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;
// TcpServer和TcpClient共用
// 用户只需要针对Connection做处理
class TcpConnection : private noncopyable,
                      public std::enable_shared_from_this<TcpConnection>
{
  public:
    enum ConnectionState
    {
        kConnected,
        kPeerHalfClose,
        kUsHalfClose,
        kDisConnecting,
        kDisconnected
    };

    typedef std::function<void(TcpConnectionPtr)> EventCallback;
    typedef std::function<void(TcpConnectionPtr, Buffer &, mTimestamp)>
                                  MessageFunc;
    typedef std::function<void()> NonInputParamCallback;

    TcpConnection(EventLoop *, Socket &&);
    ~TcpConnection();

    void set_onAcceptCallback(const NonInputParamCallback &cb)
    {
        onAcceptCallback_ = cb;
    }
    void set_onMessageCallback(const MessageFunc &cb)
    {
        onMessageCallback_ = cb;
    }
    void set_onConnectionCallback(const EventCallback &cb)
    {
        onConnectionCallback_ = cb;
    }
    void set_onWriteCompleteCallback(const EventCallback &cb)
    {
        onWriteCompleteCallback_ = cb;
    }
    void set_onCloseCallback(const NonInputParamCallback &cb)
    {
        onCloseCallback_ = cb;
    }

    int fd() const { return sock_.fd(); }

    void remove_self_in_loop();

    void send(std::string_view m);
    void send(const char *buf, size_t len);
    void shutdown();

    ConnectionState state() const { return state_; }

    void set_interest_event(int event) { interest_event_ = event; }
    int  interest_event() const { return interest_event_; }
    void handle_event(int event, mTimestamp);

    void listen_on_read_event();
    void disable_read();
    void listen_on_write_event();
    void disable_write();

    // only accept instead of read something
    void    set_server_mode() { server_mode_ = true; }
    Socket *accept() { return sock_.accept(); }

  private:
    bool writing() const { return writing_; }

    void handle_read(mTimestamp receive_time);
    void handle_write();
    void handle_error();
    void handle_close();

  private:
    int interest_event_;

    bool writing_;
    bool server_mode_;

    ConnectionState state_;
    EventLoop *     loop_;
    Socket          sock_;

    Buffer read_buffer_;
    Buffer write_buffer_;
    Mutex  wr_buffer_mutex_;

    NonInputParamCallback onAcceptCallback_;
    MessageFunc           onMessageCallback_;
    EventCallback         onConnectionCallback_;
    EventCallback         onWriteCompleteCallback_;
    NonInputParamCallback onCloseCallback_;
};

} // namespace reactor

#endif