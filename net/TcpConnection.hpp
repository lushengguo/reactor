#pragma once

#include "base/Buffer.hpp"
#include "base/mutex.hpp"
#include "base/noncopyable.hpp"
#include "base/timestamp.hpp"
#include "net/Channel.hpp"
#include <functional>
#include <memeory>
namespace reactor
{
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
        kDisconnected
    };

    typedef std::function<void()> EventCallback;
    TcpConnection(EventLoop *loop, Socket &socket);
    ~TcpConnection();

    void set_onMessageCallback(const EventCallback &cb)
    {
        onMessageCallback_ = cb;
    }
    void set_onConnectionCallback(const EventCallback &cb)
    {
        onConnectionCallback_ = cb;
    }

    size_t send(const Buffer *);
    void   shutdown_write();

    ConnectionState state() const { return state_; }

    int  interest_event() const { return interest_event_; }
    void handle_event(int event);

  private:
    //用户可能多次写
    //如果同步进行发送就会乱序，同一时间只能有一个线程在对该描述符写
    bool current_thread_writing() const;
    bool try_lock_write();
    void release_lock_write();
    bool lock_write();
    void enable_write();
    void disable_write();

    //读
    void enable_read();
    void disable_read();

    //带外数据暂时不管

    //异步回调
    void handle_read(mTimestamp receive_time);
    void handle_write();
    void handle_error();
    void hanle_close();
    void close();

  private:
    int interest_event_;

    Mutex     wrmutex_;
    bool      writing_;
    pthread_t writing_tid_;

    ConnectionState state_;
    EventLoop *     loop_;
    Socket          socket_;

    Buffer read_buffer_;
    Buffer write_buffer_;

    EventCallback onMessageCallback_;
    EventCallback onConnectionCallback_;
};

typedef std::shared_ptr<TcpConnection> TcpConnectionPtr;

} // namespace reactor
