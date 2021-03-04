#include "base/Errno.hpp"
#include "base/log.hpp"
#include "net/Buffer.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpConnection.hpp"
#include <sys/epoll.h>
#include <unistd.h>

using namespace std::placeholders;
namespace reactor
{
TcpConnection::TcpConnection(EventLoop *loop, Socket &&socket)
  : writing_(false), state_(kConnected), loop_(loop), sock_(std::move(socket))
{
    enable_read();
}

TcpConnection::~TcpConnection() {}

void TcpConnection::send(char *buf, size_t len)
{
    // IO都放到loop线程做 因为IO只是把数据拷贝到内核
    //内存带宽远远大于网络带宽 这对网络数据收发性能无影响
    //而且解决了读写同步问题
    loop_->assert_in_loop_thread();
    if (buf == nullptr || len == 0)
        return;

    // IO只会在loop线程做 所以不用考虑同步问题
    if (write_buffer_.readable_bytes() == 0)
    {
        int r = sock_.write(buf, len);
        if (r >= 0 && (static_cast<size_t>(r) <= len))
        {
            write_buffer_.append(buf + r, len - r);
        }
        else
        {
            handle_error();
        }
    }
}

void TcpConnection::remove_self_in_loop()
{
    loop_->remove_connection(shared_from_this());
}

void TcpConnection::shutdown_write() { sock_.shutdown(); }

void TcpConnection::enable_read()
{
    interest_event_ |= EPOLLIN | EPOLLPRI;
    loop_->update_connection(shared_from_this());
}

void TcpConnection::disable_read()
{
    interest_event_ &= (~(EPOLLIN | EPOLLPRI));
    loop_->update_connection(shared_from_this());
}

void TcpConnection::enable_write()
{
    interest_event_ |= EPOLLOUT;
    loop_->update_connection(shared_from_this());
}

void TcpConnection::disable_write()
{
    interest_event_ &= (~EPOLLOUT);
    loop_->update_connection(shared_from_this());
}

void TcpConnection::handle_read(mTimestamp receive_time)
{
    loop_->assert_in_loop_thread();
    if (state_ == kDisConnecting || state_ == kDisconnected)
        return;

    char buffer[65535];
    int  r = sock_.read(buffer, 65535);

    if (r > 0)
    {
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

//这里要考虑的问题是 -
//程序运行在非阻塞下,如果write一直不成功（即对方不接收数据而且把缓冲区设的很小）
//那么这个操作会一直占用一个线程
//如果用户频繁的调用Send函数 那么会将任务队列填满 即使后面增加了时间轮这个功能
//也会有几秒钟的时间无法提供服务 写到这里我意识到这可能是一种DOS？
//在stack overflow搜相关的防范措施看到一句话
// I don't think there is any 100% effective software solution to DOS attacks in
// general
//只考虑用户不是恶意攻击的情况 长时间无法接收数据的用户
//应该将他本能接收的后续数据丢弃 而不是让他们占着茅坑不拉屎

//用户调用Send函数一定是在loop线程 handle_write也在loop线程 所以不用考虑🔒
void TcpConnection::handle_write()
{
    loop_->assert_in_loop_thread();

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
            loop_->run_in_queue(onWriteCompleteCallback_);
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
    loop_->assert_in_loop_thread();

    if (write_buffer_.readable_bytes() == 0)
        remove_self_in_loop();
}

void TcpConnection::handle_error()
{
    log_error("TcpConnection handle error:%s", strerror(errno));
}

void TcpConnection::handle_event(int event, mTimestamp t)
{

    if (event & EPOLLERR)
    {
        loop_->run_in_queue(
          std::bind(&TcpConnection::handle_error, shared_from_this()));
    }

    if (event & EPOLLHUP && !(event & EPOLLIN))
    {
        loop_->run_in_queue(
          std::bind(&TcpConnection::handle_close, shared_from_this()));
    }

    if (event & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        loop_->run_in_queue(
          std::bind(&TcpConnection::handle_read, shared_from_this(), t));
    }

    if (event & EPOLLOUT)
    {
        loop_->run_in_queue(
          std::bind(&TcpConnection::handle_write, shared_from_this()));
    }
}
} // namespace reactor