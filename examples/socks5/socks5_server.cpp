#include "base/log.hpp"
#include "base/mixed.hpp"
#include "net/EventLoop.hpp"
#include "net/INetAddr.hpp"
#include "net/TcpClient.hpp"
#include "net/TcpServer.hpp"
#include "socks_protocol.hpp"

//长连接 要求客户端关闭连接时能检测到
using namespace reactor;
using namespace std::placeholders;
using namespace socks5;

namespace socks5
{
class Socks5Server
{
  private:
    enum class Status
    {
        kHandshaking,
        kRequesting,
        kRelaying
    };

    bool handlePeerShutdown(TcpConnectionPtr conn, Buffer &buffer)
    {
        if (prev_readable_len_ == buffer.readable_bytes_len())
        {
            conn->shutdown();
            return true;
        }
        prev_readable_len_ = buffer.readable_bytes_len();
        return false;
    }

    void handleHandshake(TcpConnectionPtr conn, Buffer &buffer)
    {
        size_t len = buffer.readable_bytes_len();
        if (len < sizeof(ClientHandshake))
            return;

        ClientHandshake *handshake = (ClientHandshake *)buffer.readable_data();
        if (len < sizeof(ClientHandshake) + handshake->nmethods)
            return;

        ServerHandshake server_handshake;
        server_handshake.ver = 5;
        if (handshake->ver != 5)
        {
            log_error("server only support socks5");
            goto error;
        }

        if (handshake->nmethods == 0)
        {
            log_error("input methods len is zero");
            goto error;
        }

        for (size_t i = 0; i < handshake->nmethods; i++)
        {
            switch (static_cast<Method>(handshake->methods[i]))
            {
                // todo: 支持其他鉴权方式
            case Method::kNoAuthenticationRequired:
                server_handshake.method = static_cast<uint8_t>(Method::kNoAuthenticationRequired);
                break;
            default:
                goto error;
            }
        }

        conn->send((const char *)&server_handshake, sizeof(server_handshake));
        status_ = Status::kRequesting;
        return;

    error:
        server_handshake.method = static_cast<uint8_t>(Method::kNoAcceptableMethods);
        conn->send((const char *)&server_handshake, sizeof(server_handshake));
        conn->shutdown();
    }

    ReplyField connectToServer(const char *ip, uint16_t port)
    {
        INetAddr echo_server_addr(ip, port);
        client_ = std::make_unique<TcpClient>(&loop_, echo_server_addr, "Socks5ClientForEchoServer");
        bool started = client_->start();
        serverConn_ = client_->getConn();
        return started ? ReplyField::kSucceeded : ReplyField::kNetworkUnreachable;
    }

    void handleConnection(TcpConnectionPtr conn, Buffer &buffer)
    {
        size_t len = buffer.readable_bytes_len();
        if (len < sizeof(Request))
            return;

        char reply_buffer[sizeof(Reply) + 4 + 2];
        Reply *reply = (Reply *)reply_buffer;

        Request *request = (Request *)buffer.readable_data();

#define check_request(expr, ...)                                                                                       \
    if (expr)                                                                                                          \
    {                                                                                                                  \
        log_error(__VA_ARGS__);                                                                                        \
        conn->send(reply_buffer, sizeof(reply_buffer));                                                                \
        conn->shutdown();                                                                                              \
        return;                                                                                                        \
    }

        check_request(request->ver != 5, "only support socks5");
        check_request(request->cmd != static_cast<uint8_t>(Cmd::kConnect), "only support connect cmd");
        check_request(request->rsv != 0, "reserve field should keep zero");
        check_request(request->atyp != static_cast<uint8_t>(AddressType::kIpv4), "only support ipv4 address type");

#undef check_request

        if (len < sizeof(Request) + 4 + 2)
            return;

        uint32_t remote_ip = *(uint32_t *)(request + sizeof(Request));
        uint16_t remote_port = *(uint16_t *)(request + sizeof(Request) + 4);
        ReplyField field = connectToServer(number_2_ipv4(remote_ip).c_str(), remote_port);
        reply->ver = 5;
        reply->rep = static_cast<uint8_t>(field);
        reply->rsv = 0;
        reply->atyp = static_cast<uint8_t>(AddressType::kIpv4);

        memcpy(reply + sizeof(Reply), request + sizeof(Request), 4 + 2);
        if (field != ReplyField::kSucceeded)
        {
            conn->send(reply_buffer, sizeof(reply_buffer));
            conn->shutdown();
            return;
        }

        conn->send(reply_buffer, sizeof(reply_buffer));
    }

    void handleReplay(TcpConnectionPtr conn, Buffer &buffer)
    {
        if (conn == clientConn_)
            serverConn_->send(buffer.readable_data(), buffer.readable_bytes_len());
        else
            clientConn_->send(buffer.readable_data(), buffer.readable_bytes_len());
    }

  private:
    typedef TcpConnectionPtr EchoClientConn;
    typedef TcpConnectionPtr EchoServerConn;

    EchoClientConn clientConn_;
    EchoServerConn serverConn_;
    EventLoop &loop_;
    std::unique_ptr<TcpClient> client_;

    Status status_ = Status::kHandshaking;

    size_t prev_readable_len_;

  public:
    Socks5Server(EventLoop &loop) : loop_(loop), prev_readable_len_(0) {}

    void onMessage(TcpConnectionPtr conn, Buffer &buffer, MilliTimestamp receive_timestamp)
    {
        if (handlePeerShutdown(conn, buffer))
            return;

        if (not clientConn_)
            clientConn_ = conn;

        switch (status_)
        {
        case Status::kHandshaking:
            handleHandshake(conn, buffer);
            break;
        case Status::kRequesting:
            handleConnection(conn, buffer);
            break;
        case Status::kRelaying:
            handleReplay(conn, buffer);
            break;
        default:
            break;
        }
    }
};

}; // namespace socks5

int main(int argc, char **argv)
{
    EventLoop loop;
    Socks5Server *socks5_server = new Socks5Server(loop);
    INetAddr addr("", 1080);
    TcpServer server(&loop, addr, "Socks5Server");

    server.set_onMessageCallback(std::bind(&Socks5Server::onMessage, socks5_server, _1, _2, _3));

    server.start();
    loop.loop();
}