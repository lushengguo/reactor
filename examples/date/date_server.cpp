#include "base/log.hpp"
#include "base/timestamp.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpServer.hpp"
namespace reactor
{
class DateServer
{
  public:
    void onConnection(TcpConnectionPtr conn)
    {
        std::string date = readable_current_time();
        conn->send(date.c_str(), date.size());
        conn->shutdown();
    }
};

}; // namespace reactor

//短连接 要求连接建立后发送日期信息然后主动关掉连接
using namespace reactor;
using namespace std::placeholders;
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        log_error("Usage : ./date-server <port>");
        exit(-1);
    }

    DateServer *date = new DateServer;
    EventLoop   loop;
    loop.init();
    INetAddr  addr("", atoi(argv[1]));
    TcpServer server(&loop, addr, "DateServer");

    server.set_onConnectionCallback(
      std::bind(&DateServer::onConnection, date, _1));

    server.start();
    loop.loop();
}