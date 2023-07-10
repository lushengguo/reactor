#include "net/EventLoop.hpp"
#include "net/TcpClient.hpp"
#include "protocol.hpp"
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <string_view>

using namespace reactor;
using namespace std::placeholders;

namespace reactor
{
class RedisClient : RedisProtocol
{
  public:
    RedisClient(EventLoop *loop, std::string_view ip) : client(loop, INetAddr(ip, 6379), "RedisClient") {}

    void send(std::string_view cmd)
    {
        if (conn_)
            conn_->send(cmd);
    }

    void start()
    {
        client.set_onConnectionCallback(std::bind(&RedisClient::onConnection, this, _1));
        client.set_onMessageCallback(std::bind(&RedisClient::onMessage, this, _1, _2, _3));
        client.start();
    }

  private:
    void onConnection(TcpConnectionPtr conn)
    {
        log_trace("connection established");
        conn_ = conn;
    }

    void onMessage(TcpConnectionPtr conn, Buffer &buffer, MilliTimestamp receive_timestamp)
    {
        log_trace("on message callback");
        std::string response;
        size_t len = 0;
        while ((len = parse_response(std::string(buffer.read_all_as_string()), response)) != 0)
        {
            std::cout << response << std::endl;
            buffer.retrive(len);
        }
    }

  private:
    TcpClient client;
    TcpConnectionPtr conn_;
};
} // namespace reactor

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./" << argv[0] << " ip" << std::endl;
        exit(-1);
    }

    // disable_log_print();
    EventLoop loop;
    RedisClient client(&loop, argv[1]);
    client.start();
    // read from standard input as cmd
    loop.run_in_work_thread([&]() {
        while (true)
        {
            std::string line;
            std::getline(std::cin, line);
            line.append("\r\n");
            client.send(line);
        }
    });
    // start event loop
    loop.loop();
}
