#include "base/Error.hpp"
#include "net/TcpConnection.hpp"
#include <sys/epoll.h>
#include <unistd.h>

using std::placeholders;
namespace reactor
{
TcpConnection::TcpConnection(EventLoop *loop, Socket &socket)
  : writing_(false), state_(kConnected), loop_(loop), socket_(socket)
{
    enable_read();
}

TcpConnection::~TcpConnection() {}

size_t TcpConnection::send(Buffer *content) { return handle_write(*content); }

void TcpConnection::shutdown_write()
{
    state_ = kUsHalfClose;
    socket_.shutdown_write();
}

void TcpConnection::close()
{
    state_ = kDisconnected;
    socket_.close();
}

void TcpConnection::enable_read()
{
    interest_event_ |= EPOLLRDNORM;
    loop_->update_connection(shared_from_this());
}

void TcpConnection::disable_read()
{
    interest_event_ &= (~EPOLLRDNORM);
    loop_->update_connection(shared_from_this());
}

void TcpConnection::enable_write()
{
    interest_event_ |= EPOLLWRNORM;
    loop_->update_connection(shared_from_this());
}

void TcpConnection::disable_write()
{
    interest_event_ &= (~EPOLLWRNORM);
    loop_->update_connection(shared_from_this());
}

bool TcpConnection::current_thread_writing() const
{
    return writing_tid_ == pthread_self();
}

bool TcpConnection::try_lock_write()
{
    if (writing_ && current_thread_writing())
    {
        return true;
    }
    else if (!writing_)
    {
        return lock_write();
    }
    else
    {
        return false;
    }
}

void TcpConnection::release_lock_write()
{
    if (current_thread_writing())
    {
        writing_     = false;
        writing_tid_ = 0;
    }
}

bool TcpConnection::lock_write()
{
    assert(!writing_);
    MutexLockGuard lock(wrmutex_);
    writing_     = true;
    writing_tid_ = pthread_self();
}

void TcpConnection::handle_read(mTimestamp receive_time)
{
    ErrorCode err;
    socket_.read_into_Buffer(read_buffer_, err);
    size_t rdlen = read_buffer_.readable_bytes();

    if (rdlen > 0)
    {
        messageCallback_(shared_from_this(), &read_buffer_, receive_time);
    }
    else if (rdlen == 0 && !err)
    {
        state_ = kPeerHalfClose;
        handleClose();
    }
    else
    {
        state_ = kDisconnected;
        handleError(err);
    }
}

void TcpConnection::handle_write(Buffer &buffer)
{
    if (buffer.readable_bytes() == 0 || !try_lock_write())
        return;

    if (state_ == kPeerHalfClose || state_ == kConnected)
    {
        ErrorCode err;
        size_t r = socket_.write(buffer.data(), buffer.readable_bytes(), err);
        if (r == buffer.readable_bytes())
        {
            release_lock_write();
        }

        //状态为EPOLLWRNORM才会调用这个函数
        if (r > 0)
        {
            buffer.retrive(r);
        }
        else
        {
            state_ = kDisconnected;
            handle_error(err);
        }
    }
}

void TcpConnection::handle_close() { close(); }

void TcpConnection::handle_error(ErrorCode err)
{
    log_error("TcpConnection handle error:%s", strerror(err));
}

void TcpConnection::handle_event(int event, mTimestamp t)
{
    if (event & EPOLLRDNORM)
    {
        loop_->run_in_queue(
          std::bind(&TcpConnection::handle_read(), shared_from_this(), t));
    }

    if (event & EPOLLWRNORM)
    {
        loop_->run_in_queue(
          std::bind(&TcpConnection::handle_write(), shared_from_this()));
    }
}
} // namespace reactor