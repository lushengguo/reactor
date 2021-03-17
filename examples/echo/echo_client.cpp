#include "base/log.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpClient.hpp"
#include <iostream>
#include <string>
#include <string_view>
namespace reactor
{
class EchoClient
{
  public:
    //连接建立后主动由client发起数据通讯
    void onConnection(TcpConnectionPtr conn)
    {
        std::cout << "please input message >> " << std::flush;
        std::getline(std::cin, s_);
        conn->send(s_);
    }

    void onMessage(
      TcpConnectionPtr conn, Buffer &buffer, mTimestamp receive_timestamp)
    {
        std::cout << "recv from server << "
                  << buffer.read_all_as_string().data() << std::endl;
        buffer.retrive_all();

        std::cout << "please input message >> " << std::flush;
        std::getline(std::cin, s_);
        conn->send(s_);
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
        log_error("Usage : ./echo-client <ip> <port>");
        exit(-1);
    }

    EchoClient *echo = new EchoClient;
    EventLoop   loop;
    INetAddr    addr(argv[1], atoi(argv[2]));
    TcpClient   client(&loop, addr, "EchoClient");

    client.set_onConnectionCallback(
      std::bind(&EchoClient::onConnection, echo, _1));
    client.set_onMessageCallback(
      std::bind(&EchoClient::onMessage, echo, _1, _2, _3));
    client.start();
    loop.loop();
}