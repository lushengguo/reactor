#include "base/condition.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpClient.hpp"
#include "protocol.hpp"
#include <assert.h>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <stdio.h>
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
    RedisClient(EventLoop *loop, std::string_view ip)
      : client_(loop, INetAddr(ip, 6379), "RedisClient")
    {}

    void send(std::string_view cmd)
    {
        if (conn_)
            conn_->send(cmd);
    }

    void start()
    {
        client_.set_onConnectionCallback(
          std::bind(&RedisClient::onConnection, this, _1));
        client_.set_onMessageCallback(
          std::bind(&RedisClient::onMessage, this, _1, _2, _3));
        client_.start();
    }

    void        wait() { cond_.wait(); }
    void        signal() { cond_.signal(); }
    std::string response() { return response_; }
    bool        connection_established() { return conn_ != nullptr; }

  private:
    void onConnection(TcpConnectionPtr conn)
    {
        log_trace("connection established");
        conn_ = conn;
    }

    void onMessage(TcpConnectionPtr conn, Buffer &buffer, MicroTimeStamp receive_timestamp)
    {
        size_t len = 0;
        response_  = "";
        while ((len = parse_response(std::string(buffer.read_all_as_string()),
                  response_)) != 0)
        {
            buffer.retrive(len);
            signal();
        }
    }

  private:
    TcpClient        client_;
    TcpConnectionPtr conn_;
    std::string      response_;
    Condition        cond_;
};
} // namespace reactor

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " ip" << std::endl;
        exit(-1);
    }

    disable_log_print();
    EventLoop   loop;
    RedisClient client(&loop, argv[1]);
    // connect server
    client.start();
    // test function
    std::function<void(std::string, std::string)> TEST =
      [&](std::string input, std::string expect_response) {
          printf("input:[%s] expect-response:[%s] ",
            replace_all(input, "\r\n", "").c_str(),
            expect_response.c_str());

          client.send(input);
          client.wait();
          printf("real-response[%s]\n", client.response().c_str());
          assert(expect_response == client.response());
      };

    loop.run_in_work_thread([&]() {
        TEST("set number 1\r\n", "OK");
        TEST("get number\r\n", "1");
        TEST("set name Barak\r\n", "OK");
        TEST("get name\r\n", "Barak");
        // TEST("set number 1\r\n", "OK");
        // TEST("set number 1\r\n", "OK");
        // TEST("set number 1\r\n", "OK");
        // TEST("set number 1\r\n", "OK");
        // TEST("set number 1\r\n", "OK");
        // TEST("set number 1\r\n", "OK");
        exit(0);
    });
    loop.loop();
}
