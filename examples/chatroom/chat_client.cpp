#include "base/log.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpClient.hpp"
#include "protocol.hpp"
#include <functional>
#include <iostream>
#include <string_view>

using namespace std::placeholders;
namespace reactor
{
class ChatClient
{
  public:
    ChatClient(EventLoop *loop, INetAddr addr) : client(loop, addr, "ChatClient") {}

    void onMessage(TcpConnectionPtr conn, Buffer &buffer, MilliTimestamp receive_timestamp)
    {
        std::cout << buffer.read_all_as_string() << std::endl;
        buffer.retrive_all();
    }

    void onConnection(TcpConnectionPtr conn) { conn_ = conn; }

    void start()
    {
        client.set_onConnectionCallback(std::bind(&ChatClient::onConnection, this, _1));
        client.set_onMessageCallback(std::bind(&ChatClient::onMessage, this, _1, _2, _3));
        client.start();
    }

    void send(std::string_view m) { conn_->send(m); }

    bool connection_build() { return conn_ != nullptr; }

  private:
    TcpConnectionPtr conn_;
    TcpClient client;
    Protocal protocal_;
    std::string input_;
};
} // namespace reactor

using namespace reactor;
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./" << argv[0] << " ip port" << std::endl;
        exit(-1);
    }

    disable_log_print();
    EventLoop loop;
    INetAddr addr(argv[1], atoi(argv[2]));
    ChatClient client(&loop, addr);
    client.start();
    loop.run_in_work_thread([&]() {
        while (!client.connection_build())
        {
        }
        while (true)
        {
            std::string line;
            std::getline(std::cin, line);
            client.send(line);
        }
    });
    loop.loop();
}