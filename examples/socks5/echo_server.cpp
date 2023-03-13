#include "base/log.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpServer.hpp"
namespace reactor
{
class EchoServer
{
  public:
    void onMessage(TcpConnectionPtr conn, Buffer &buffer, MicroTimeStamp receive_timestamp)
    {
        log_trace("recv data:%s, len=%d",
          buffer.read_all_as_string().data(),
          buffer.readable_bytes());
        conn->send(buffer.read_all_as_string());
        buffer.retrive_all();
    }
};

}; // namespace reactor

//长连接 要求客户端关闭连接时能检测到
using namespace reactor;
using namespace std::placeholders;
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        log_error("Usage : ./echo-server <port>");
        exit(-1);
    }

    EchoServer *echo = new EchoServer;
    EventLoop   loop;
    INetAddr    addr("", atoi(argv[1]));
    TcpServer   server(&loop, addr, "EchoServer");

    server.set_onMessageCallback(
      std::bind(&EchoServer::onMessage, echo, _1, _2, _3));

    server.start();
    loop.loop();
}