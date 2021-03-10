#include "base/Buffer.hpp"
#include "base/Errno.hpp"
#include "base/log.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpConnection.hpp"
#include <sys/epoll.h>
#include <unistd.h>

using namespace std::placeholders;
namespace reactor
{
TcpConnection::TcpConnection(EventLoop *loop, Socket &&socket)
  : writing_(false),
    server_mode_(false),
    state_(kConnected),
    loop_(loop),
    sock_(std::move(socket))
{}

TcpConnection::~TcpConnection()
{
    log_info("close connection with ip=%s,port=%d",
      sock_.readable_ip().data(),
      sock_.hostport());
}

void TcpConnection::send(const char *buf, size_t len)
{
    // IO都放到loop线程做 因为IO只是把数据拷贝到内核
    //内存带宽远远大于网络带宽 这对网络数据收发性能无影响
    //而且解决了读写同步问题
    if (buf == nullptr || len == 0)
        return;

    // IO只会在loop线程做 所以不用考虑同步问题
    if (write_buffer_.readable_bytes() == 0)
    {
        assert(!writing());
        int r = sock_.write(buf, len);
        // log_trace("write %d bytes to fd=%d", r, sock_.fd());
        if (r >= 0)
        {
            size_t sr = r;
            if (sr < len)
            {
                enable_write();
                write_buffer_.append(buf + r, len - sr);
            }
        }
        else
        {
            handle_error();
        }
    }
    else
    {
        write_buffer_.append(buf, len);
    }
}

void TcpConnection::remove_self_in_loop()
{
    loop_->remove_monitor_object(shared_from_this());
}

void TcpConnection::shutdown()
{
    sock_.shutdown();
    remove_self_in_loop();
}

void TcpConnection::enable_read()
{
    interest_event_ |= EPOLLIN | EPOLLPRI;
    loop_->update_monitor_object(shared_from_this());
}

void TcpConnection::disable_read()
{
    interest_event_ &= (~(EPOLLIN | EPOLLPRI));
    loop_->update_monitor_object(shared_from_this());
}

void TcpConnection::enable_write()
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

void TcpConnection::handle_read(mTimestamp receive_time)
{
    if (server_mode_)
    {
        if (onAcceptCallback_)
            onAcceptCallback_();
    }
    else
    {
        char buffer[65535];
        int  r = sock_.read(buffer, 65535);

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

    if (write_buffer_.readable_bytes() == 0)
    {
        disable_write();
        return;
    }

    int r = sock_.write(write_buffer_.readable_data(),
      write_buffer_.readable_bytes());

    if (r >= 0)
    {
        size_t sr = r;
        write_buffer_.retrive(sr);
        if (sr == write_buffer_.readable_bytes())
        {
            loop_->run_in_work_thread(
              std::bind(onWriteCompleteCallback_, shared_from_this()));
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
    if (write_buffer_.readable_bytes() == 0)
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

void TcpConnection::handle_event(int event, mTimestamp t)
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