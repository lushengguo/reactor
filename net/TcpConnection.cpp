#include "net/TcpConnection.hpp"
#include "base/Buffer.hpp"
#include "base/Errno.hpp"
#include "base/log.hpp"
#include "net/EventLoop.hpp"
#include <sys/epoll.h>
#include <unistd.h>

using namespace std::placeholders;
namespace reactor
{
TcpConnection::TcpConnection(EventLoop *loop, Socket &&socket)
    : writing_(false), server_mode_(false), state_(kConnected), loop_(loop), sock_(std::move(socket))
{
}

TcpConnection::~TcpConnection() { log_info("close connection with ip=%s,port=%d", sock_.readable_ip().data(), sock_.hostport()); }

void TcpConnection::send(std::string_view m) { send(m.data(), m.size()); }

void TcpConnection::send(const char *buf, size_t len)
{
    if (buf == nullptr || len == 0)
        return;

    // IO都放到loop线程做 因为IO只是把数据拷贝到内核
    //内存带宽远远大于网络带宽 这对网络数据收发性能无影响
    //而且解决了读写同步问题
    if (loop_->in_loop_thread())
    {
        if (write_buffer_.readable_bytes_len() == 0)
        {
            int r = sock_.write(buf, len);
            log_trace("write %d bytes to fd=%d", r, sock_.fd());
            if (r < 0)
                handle_error();

            // remain bytes
            size_t sr = r;
            if (sr < len)
            {
                log_debug("send incomplete ,save to write buffer");
                listen_on_write_event();
                write_buffer_.append(buf + r, len - sr);
            }
        }
        else
        {
            log_debug("write buffer is not empty, send incomplete ,save to "
                      "write buffer");
            listen_on_write_event();
            write_buffer_.append(buf, len);
        }
    }
    else
    {
        log_debug("not in loop thread, send failed, save data to write buffer");
        //存储到write_buffer_里 由wr回调负责全部发送出去
        listen_on_write_event();
        MutexLockGuard lock(wr_buffer_mutex_);
        write_buffer_.append(buf, len);
    }
}

void TcpConnection::remove_self_in_loop() { loop_->remove_monitor_object(shared_from_this()); }

void TcpConnection::shutdown()
{
    sock_.shutdown();
    remove_self_in_loop();
}

void TcpConnection::listen_on_read_event()
{
    interest_event_ |= EPOLLIN | EPOLLPRI;
    loop_->update_monitor_object(shared_from_this());
}

void TcpConnection::disable_read()
{
    interest_event_ &= (~(EPOLLIN | EPOLLPRI));
    loop_->update_monitor_object(shared_from_this());
}

void TcpConnection::listen_on_write_event()
{
    if (writing())
        return;

    writing_ = true;
    interest_event_ |= EPOLLOUT;
    loop_->update_monitor_object(shared_from_this());
}

void TcpConnection::disable_write()
{
    if (!writing())
        return;

    writing_ = false;
    interest_event_ &= (~EPOLLOUT);
    loop_->update_monitor_object(shared_from_this());
}

void TcpConnection::handle_read(MilliTimestamp receive_time)
{
    if (server_mode_)
    {
        if (onAcceptCallback_)
            onAcceptCallback_();
    }
    else
    {
        char buffer[65535];
        int r = sock_.read(buffer, 65535);

        if (r > 0)
        {
            // log_trace("recv %d bytes from peer", r);
            read_buffer_.append(buffer, static_cast<size_t>(r));
            onMessageCallback_(shared_from_this(), read_buffer_, receive_time);
        }
        else if (r == 0)
        {
            // peer shutdown connection
            handle_close();
        }
        else
        {
            handle_error();
        }
    }
}

//用户调用Send函数一定是在loop线程 handle_write也在loop线程 所以不用考虑🔒
//不管是send还是handle_write 如果写不成功就把剩下的数据存起来
void TcpConnection::handle_write()
{
    assert(writing());
    MutexLockGuard lock(wr_buffer_mutex_);
    if (write_buffer_.readable_bytes_len() == 0)
    {
        disable_write();
        return;
    }

    //主线程负责写暂存未发送出去的内容 send函数只是负责发一次
    //全部发完由回调处理
    log_debug("write buffer non-empty,send in write event handler");
    log_debug("data in write buffer:[%s]", write_buffer_.read_all_as_string().data());
    int r = sock_.write(write_buffer_.readable_data(), write_buffer_.readable_bytes_len());
    log_debug("send %d bytes in write event handler", r);

    if (r >= 0)
    {
        size_t sr = r;
        write_buffer_.retrive(sr);
        if (sr == write_buffer_.readable_bytes_len())
        {
            loop_->run_in_work_thread(std::bind(onWriteCompleteCallback_, shared_from_this()));
            disable_write();
        }
    }
    else
    {
        handle_error();
    }
}

//对方半关闭连接后调用这个函数 应该把要发的数据发完本端再关闭连接
void TcpConnection::handle_close()
{
    if (write_buffer_.readable_bytes_len() == 0)
    {
        remove_self_in_loop();
        if (onCloseCallback_)
            onCloseCallback_();
    }
}

void TcpConnection::handle_error()
{
    log_error("TcpConnection handle error:%s", strerror(errno));
    remove_self_in_loop();
    if (onCloseCallback_)
        onCloseCallback_();
}

void TcpConnection::handle_event(int event, MilliTimestamp t)
{
    if (event & EPOLLERR)
    {
        handle_error();
    }

    if (event & EPOLLHUP && !(event & EPOLLIN))
    {
        handle_close();
    }

    if (event & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        handle_read(t);
    }

    if (event & EPOLLOUT)
    {
        handle_write();
    }
}
} // namespace reactor