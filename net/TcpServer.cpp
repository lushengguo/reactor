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
    Socket newsock;
    if (sock_.accept(newsock))
    {
        auto p = std::make_shared<TcpConnection>(this, newsock);
        p->set_onConnectionCallback(onConnectionCallback_);
        p->set_onMessageCallback(onMessageCallback_);
        p->set_onWriteCompleteCallback(onWriteCompleteCallback_);
        loop_->update_connection(p);
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

    self_.reset(std::make_shared<TcpConnection>(this, sock_));
    self_.set_interest_events(EPOLLIN);
    // server的生存时间应该和启动进程一样长
    // 所以server还是以裸指针方式传递
    self_.set_onMessageCallback(std::bind(&TcpServer::accept, this));

    //上面有一个问题 - 每一个通讯的单位应该是Channel
    //但是我全部把他们的所有接口都写到TcpConnection里了
    //这样分类不是太规范
    //必须把轮询accept事件当成是一个connecton丢到loop里
    loop_->update_connection(self_);
}

} // namespace reactor