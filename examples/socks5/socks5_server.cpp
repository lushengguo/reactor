#include "base/log.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpServer.hpp"
namespace reactor
{
class Socks5Server
{
  public:
    void onMessage(TcpConnectionPtr conn, Buffer &buffer, MicroTimeStamp receive_timestamp)
    {
        log_trace("recv data:%s, len=%d", buffer.read_all_as_string().data(), buffer.readable_bytes());
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
    Socks5Server *socks5_server = new Socks5Server;
    EventLoop loop;
    INetAddr addr("", 1080);
    TcpServer server(&loop, addr, "Socks5Server");

    server.set_onMessageCallback(std::bind(&Socks5Server::onMessage, socks5_server, _1, _2, _3));

    server.start();
    loop.loop();
}