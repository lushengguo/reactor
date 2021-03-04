#include "base/log.hpp"
#include "net/TcpConnection.hpp"
#include "net/TcpServer.hpp"
#include <assert.h>
#include <sys/epoll.h>

namespace reactor
{

TcpServer::~TcpServer()
{
    if (self_connection_)
    {
        self_connection_->remove_self_in_loop();
    }
}

void TcpServer::accept()
{
    loop_->assert_in_loop_thread();

    //临时创建socket 然后std::move给TcpConnection
    //连接的生命期由TcpConnection管理 析构时自动关闭连接
    Socket *newsock = self_connection_->accept();
    if (newsock)
    {
        log_info("Server %s accept new connection,ip=%s,port=%d",
          name_.c_str(),
          newsock->readable_ip().data(),
          newsock->hostport());
        // server类的职责只有这些
        //把用户注册的事件处理方式转发给每一个Connection
        //当事件来临时 TcpConnection调用这些回调处理事件
        //具体逻辑都在TcpConnection内 其码同时由TcpClient复用
        auto conn = std::make_shared<TcpConnection>(loop_, std::move(*newsock));
        conn->set_onConnectionCallback(onConnectionCallback_);
        conn->set_onMessageCallback(onMessageCallback_);
        conn->set_onWriteCompleteCallback(onWriteCompleteCallback_);
        conn->enable_read();
    }
}

void TcpServer::start()
{
    //非线程安全 server设计上就是应该只启动一次
    if (started_)
        return;

    loop_->init_poller();
    started_ = true;

    sock_.set_reuse_addr();
    assert(sock_.bind(endpoint_));
    assert(sock_.listen());

    // server的生命必然长于connection
    self_connection_ = std::make_shared<TcpConnection>(loop_, std::move(sock_));
    self_connection_->set_interest_event(EPOLLIN);
    self_connection_->set_server_mode();
    self_connection_->set_onAcceptCallback(std::bind(&TcpServer::accept, this));

    //上面有一个问题 - 每一个通讯的单位应该是Channel
    //但是我全部把他们的所有接口都写到TcpConnection里了
    //不得不在TcpConnection里开后门 支持server-mode
    self_connection_->enable_read();
    log_info("TcpServer register listening in loop finish");
}

} // namespace reactor