#include "net/TcpServer.hpp"
#include <assert.h>

namespace reactor
{

TcpServer::TcpServer(EventLoop *loop, const INetAddr &addr)
  : sock_(new Socket())
{
    loop_.reset(loop);
}

int TcpServer::set_thread_num(size_t n)
{
    if (ThreadPool_ == nullptr)
    {
        ThreadPool_.reset(new ThreadPool(2 * n, n));
        ThreadPool_->start();
    }

    return ThreadPool_->thread_num;
}

void TcpServer::refuse_connection(int fd) const { close(fd); }

void TcpServer::new_connection(int fd)
{
    TcpConnection *conn = new TcpConnection();
    conn->set_onMessageCallback(onMessageCallback_);
    conn->set_onConnectionCallback(onConnectionCallback_);
}

void TcpServer::wait_connection()
{
    assert(sock_->Bind(addr_));
    while (true)
    {
        assert(sock_->Listen());
        int peer = sock_->Accept();
        assert(peer);
        new_connection(peer);
    }
}

void TcpServer::start() { wait_connection(); }

TcpServer::TcpServer(EventLoop *loop, const INetAddr &addr)
  : sock_(new Socket()), addr_(addr)
{
    loop_.reset(loop);
}

} // namespace reactor