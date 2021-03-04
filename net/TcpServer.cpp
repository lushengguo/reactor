#include "base/log.hpp"
#include "net/TcpConnection.hpp"
#include "net/TcpServer.hpp"
#include <assert.h>
#include <sys/epoll.h>

namespace reactor
{

TcpServer::~TcpServer()
{
    if (self_)
    {
        self_->remove_self_in_loop();
    }
}

void TcpServer::accept()
{
    ploop_->assert_in_loop_thread();

    //临时创建socket 然后std::move给TcpConnection
    //连接的生命期由TcpConnection管理 析构时自动关闭连接
    Socket *newsock = sock_.accept();
    if (newsock)
    {
        log_info("Server %s accept new connection,ip=%s,port=%d",
          name_.c_str(),
          newsock->readable_ip(),
          newsock->hostport());
        auto conn =
          std::make_shared<TcpConnection>(ploop_, std::move(*newsock));
        conn->set_onConnectionCallback(onConnectionCallback_);
        conn->set_onMessageCallback(onMessageCallback_);
        conn->set_onWriteCompleteCallback(onWriteCompleteCallback_);
        ploop_->update_connection(conn);
    }
}

void TcpServer::start()
{
    //非线程安全 server设计上就是应该只启动一次
    if (started_)
        return;

    started_ = true;

    assert(sock_.bind(endpoint_));
    assert(sock_.listen());
    assert(!self_);
    self_ = std::make_shared<TcpConnection>(ploop_, std::move(sock_));
    self_->set_interest_event(EPOLLIN);
    // server的生存时间应该和启动进程一样长
    // 所以server还是以裸指针方式传递
    self_->set_onMessageCallback(std::bind(&TcpServer::accept, this));

    //上面有一个问题 - 每一个通讯的单位应该是Channel
    //但是我全部把他们的所有接口都写到TcpConnection里了
    //这样分类不是太规范
    //必须把轮询accept事件当成是一个connecton丢到loop里
    ploop_->update_connection(self_);
    log_info("TcpServer register listening in loop finish");
}

} // namespace reactor