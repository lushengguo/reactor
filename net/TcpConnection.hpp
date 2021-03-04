#pragma once
#ifndef REACTOR_TCPCONNECTION_HPP
#define REACTOR_TCPCONNECTION_HPP

#include "base/mutex.hpp"
#include "base/noncopyable.hpp"
#include "base/timestamp.hpp"
#include "net/Socket.hpp"
#include <functional>
#include <memory>

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

    typedef std::function<void()> EventCallback;
    typedef std::function<void(TcpConnectionPtr, Buffer, mTimestamp)>
      MessageFunc;
    TcpConnection(EventLoop *, Socket &&);
    ~TcpConnection();

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

    int fd() const { return sock_.fd(); }

    void remove_self_in_loop();

    void send(char *buf, size_t len);
    void shutdown_write();

    ConnectionState state() const { return state_; }

    void set_interest_event(int event) { interest_event_ = event; }
    int  interest_event() const { return interest_event_; }
    void handle_event(int event, mTimestamp);

  private:
    void enable_read();
    void disable_read();
    void enable_write();
    void disable_write();

    void handle_read(mTimestamp receive_time);
    void handle_write();
    void handle_error();
    void handle_close();

  private:
    int interest_event_;

    Mutex     wrmutex_;
    Mutex     wbuffer_mutex_;
    bool      writing_;
    pthread_t writing_tid_;

    ConnectionState state_;
    EventLoop *     loop_;
    Socket          sock_;

    Buffer read_buffer_;
    Buffer write_buffer_;

    MessageFunc   onMessageCallback_;
    EventCallback onConnectionCallback_;
    EventCallback onWriteCompleteCallback_;
};

} // namespace reactor

#endif