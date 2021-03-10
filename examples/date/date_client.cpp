#include "base/log.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpClient.hpp"
#include <iostream>
#include <string>
#include <string_view>
namespace reactor
{
class DateClient
{
  public:
    void onMessage(
      TcpConnectionPtr conn, Buffer &buffer, mTimestamp receive_timestamp)
    {
        log_trace("recv data:%s, len=%d",
          buffer.string(buffer.readable_bytes()).data(),
          buffer.readable_bytes());
        buffer.retrive_all();
    }

  private:
    std::string s_;
};

}; // namespace reactor

//长连接 要求客户端关闭连接时能检测到
using namespace reactor;
using namespace std::placeholders;
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        log_error("Usage : ./date-client <ip> <port>");
        exit(-1);
    }

    DateClient *date = new DateClient;
    EventLoop   loop;
    INetAddr    addr(argv[1], atoi(argv[2]));
    TcpClient   client(&loop, addr, "DateClient");

    client.set_onMessageCallback(
      std::bind(&DateClient::onMessage, date, _1, _2, _3));

    client.start();
    loop.loop();
}