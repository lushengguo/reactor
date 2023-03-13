#include "base/log.hpp"
#include "base/noncopyable.hpp"
#include "net/TcpServer.hpp"
#include "parser.hpp"
#include <string>

namespace reactor
{
class WebServer : private noncopyable
{
  public:
    void onMessage(TcpConnectionPtr conn, Buffer &buffer, MicroTimeStamp receive_time)
    {
        ParseStatus status = parser_.parse(buffer.readable_data(), buffer.readable_data());

        if (status == kValidRequest)
        {
            std::string response = router_.response(buffer.readable_data(), buffer.readable_bytes());
            conn->send(response.data(), response.size());
            buffer.retrive_all();
        }
    }

  private:
    HttpParser parser_;
    HttpRouter router_;
};

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        log_error("Usage : ./echo-server <port>");
        exit(-1);
    }

    EchoServer *echo = new EchoServer;
    EventLoop loop;
    INetAddr addr("", atoi(argv[1]));
    TcpServer server(&loop, addr, "EchoServer");

    server.set_onMessageCallback(std::bind(&EchoServer::onMessage, echo, _1, _2, _3));

    server.start();
    loop.loop();
}
} // namespace reactor