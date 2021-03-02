#include "net/TcpServer.hpp"
#include <iostream>
#include <stdlib.h>
using namespace reactor;
using namespace std::placeholders;

//用户只需要自定义连接建立时和消息到达时的行为即可
void onConnection(TcpConnectionPtr conn)
{
    if (conn != nullptr)
    {
        conn->set_tcp_no_delay();
    }
}

void onMessage(const Buffer &buf) {}

int main(int argc, char **argv)
{
    if (argc <= 4)
    {
        std::cerr << "Usage: ./char_server ip port threadnum" << std::cend;
        return -1;
    }

    INetAddr  addr(argv[1], atoi(argv[2]));
    int       threadnum = atoi(argv[3]);
    EventLoop loop;
    TcpServer server(&loop, addr);

    server.set_onConnectionCallback(std::bind(onConnection, _1));
    server.set_onMessageCallback(std::bind(onMessage, _1));
    server.set_thread_num(threadnum);
    server.start();

    loop.loop();
}