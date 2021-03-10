#include "net/EventLoop.hpp"
#include "net/TcpClient.hpp"
namespace reactor
{
void TcpClient::start()
{
    if (started_)
        return;

    started_ = true;
    sock_.set_reuse_addr();

    if (sock_.connect(server_endpoint_))
    {
        log_info("connect server ip=%s port=%d success",
          server_endpoint_.readable_ip().data(),
          server_endpoint_.hostport());
    }
    else
    {
        log_error("connect server ip=%s port=%d failed:%s",
          server_endpoint_.readable_ip().data(),
          server_endpoint_.hostport(),
          strerror(errno));
    }

    self_connection_ = std::make_shared<TcpConnection>(loop_, std::move(sock_));
    if (onConnectionCallback_)
        onConnectionCallback_(self_connection_);
    self_connection_->set_onMessageCallback(onMessageCallback_);
    self_connection_->set_onWriteCompleteCallback(onWriteCompleteCallback_);
    if (onServerCloseCallback_)
    {
        self_connection_->set_onCloseCallback(onServerCloseCallback_); // diy
    }
    else
    {
        self_connection_->set_onCloseCallback(
          std::bind(&TcpClient::onServerCloseCallback, this)); // default
    }
    self_connection_->enable_read();
}

void TcpClient::onServerCloseCallback()
{
    log_info("server closed the connection, program quit now");
    exit(0);
}

} // namespace reactor