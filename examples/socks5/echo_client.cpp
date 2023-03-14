#include "base/log.hpp"
#include "base/mixed.hpp"
#include "examples/socks5/socks_protocol.hpp"
#include "net/EventLoop.hpp"
#include "net/TcpClient.hpp"
#include "net/TcpConnection.hpp"
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
//长连接 要求客户端关闭连接时能检测到
using namespace reactor;
using namespace socks5;
using namespace std::placeholders;

namespace socks5
{
class EchoClient
{
  public:
    EchoClient(const char *echo_server_ip, uint16_t echo_server_port)
        : echo_server_ip_(echo_server_ip), echo_server_port_(echo_server_port)
    {
    }

  private:
    enum class Status
    {
        kHandshaking,
        kRequesting,
        kRelaying
    };

    void sendRequest(TcpConnectionPtr conn)
    {
        char buffer[sizeof(Request) + 4 + 2];
        char *ip = buffer + sizeof(Request);
        char *port = ip + 4;
        Request *request = (Request *)buffer;
        request->ver = 5;
        request->cmd = static_cast<uint8_t>(Cmd::kConnect);
        request->rsv = 0;
        request->atyp = static_cast<uint8_t>(AddressType::kIpv4);

        uint32_t nip = ipv4_to_number(echo_server_ip_);
        memcpy(ip, &nip, sizeof(uint32_t));
        memcpy(port, &echo_server_port_, sizeof(echo_server_port_));
        conn->send(buffer, sizeof(Request) + 4 + 2);
    }

    void sendStdinInput(TcpConnectionPtr conn)
    {
        std::string s;
        std::cin >> s;
        conn->send(s.c_str(), s.size());
    }

    void handleHandshake(TcpConnectionPtr conn, Buffer &buffer)
    {
        if (buffer.readable_bytes_len() < sizeof(ServerHandshake))
            return;

        ServerHandshake *handshake = (ServerHandshake *)buffer.readable_data();
        if (handshake->ver != 5)
        {
            log_error("server version(%d) incorrect", handshake->ver);
            exit(-1);
        }

        if (handshake->method == static_cast<uint8_t>(Method::kNoAcceptableMethods))
        {
            log_error("server handshake return code NoAcceptableMethods");
            exit(-1);
        }

        buffer.retrive(sizeof(ServerHandshake));
        status_ = Status::kRequesting;
        sendRequest(conn);
    }

    void handleReply(TcpConnectionPtr conn, Buffer &buffer)
    {
        size_t len = buffer.readable_bytes_len();
        const size_t port_len = 2;
        if (len <= sizeof(Reply))
            return;

#define check_value(val, expect, ...)                                                                                  \
    if (val != expect)                                                                                                 \
    {                                                                                                                  \
        log_error(__VA_ARGS__);                                                                                        \
        exit(-1);                                                                                                      \
    }

        Reply *reply = (Reply *)buffer.readable_data();
        check_value(reply->ver, 5, "unsupport version (%d)", reply->ver);
        check_value(reply->rep, static_cast<uint8_t>(ReplyField::kSucceeded),
                    "connect to server failed, errorcode (%d)", reply->rep);
        check_value(reply->rsv, 0, "rsv should be set to zero");
        check_value(reply->atyp, static_cast<uint8_t>(AddressType::kIpv4), "only support ipv4 server");

#undef check_value

        switch (static_cast<AddressType>(reply->atyp))
        {
        case AddressType::kIpv4: {
            if (len <= sizeof(Reply) + 4 + port_len)
                break;

            const char *addr = buffer.readable_data() + sizeof(Reply);
            uint32_t nip = ipv4_to_number(echo_server_ip_);
            if (memcmp(&nip, addr, sizeof(nip)) != 0 ||
                memcmp(&echo_server_port_, addr + sizeof(nip), sizeof(echo_server_port_)) != 0)
            {
                log_error("socks5 connects to wrong server(%s, %d), expect (%s, %d)", number_2_ipv4(nip).c_str(),
                          *(uint16_t *)(addr + sizeof(nip)), echo_server_ip_, echo_server_port_);
                exit(-1);
            }

            break;
        }
        default:
            log_error("only support ipv4 address type");
            exit(-1);
        }

        log_info("connection between socks5-server and echo server established");

        sendStdinInput(conn);
    }

    void handleEchoData(TcpConnectionPtr conn, Buffer &buffer)
    {
        log_trace("recv data:%s, len=%d", buffer.read_all_as_string().data(), buffer.readable_bytes_len());
        sendStdinInput(conn);
    }

  public:
    //连接建立后主动由client发起数据通讯
    void onConnection(TcpConnectionPtr conn)
    {
        char buffer[3];
        ClientHandshake *handshake = (ClientHandshake *)buffer;
        handshake->ver = 5;
        handshake->nmethods = 1;
        handshake->methods[0] = static_cast<uint8_t>(Method::kNoAuthenticationRequired);
        conn->send(buffer, sizeof(buffer));
    }

    void onMessage(TcpConnectionPtr conn, Buffer &buffer, MicroTimeStamp receive_timestamp)
    {
        switch (status_)
        {
        case Status::kHandshaking:
            handleHandshake(conn, buffer);
            break;
        case Status::kRequesting:
            handleReply(conn, buffer);
            break;
        case Status::kRelaying: {
            handleEchoData(conn, buffer);
            break;
        }
        default:
            exit(-1);
        }
    }

  private:
    Status status_ = Status::kHandshaking;
    const char *echo_server_ip_;
    uint16_t echo_server_port_;
};

}; // namespace socks5

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        log_error("Usage : ./echo_socks5_client <socks5-server-ip> <server-ip> <server-port> ");
        exit(-1);
    }

    const char *socks5_server_ip = argv[1];
    const char *echo_server_ip = argv[2];
    if (not verify_ipv4(echo_server_ip))
    {
        log_error("input ehco server invalid, expect ipv4 format");
        exit(-1);
    }
    uint16_t echo_server_port = atoi(argv[3]);

    EchoClient *echo = new EchoClient(echo_server_ip, echo_server_port);
    EventLoop loop;
    INetAddr addr(socks5_server_ip, 1080);
    TcpClient client(&loop, addr, "EchoClient");

    client.set_onConnectionCallback(std::bind(&EchoClient::onConnection, echo, _1));
    client.set_onMessageCallback(std::bind(&EchoClient::onMessage, echo, _1, _2, _3));
    client.start();
    loop.loop();
}