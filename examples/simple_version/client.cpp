#include <arpa/inet.h>
#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <string_view>
#include <strings.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

static std::string username;
static int fd;
static int server_fd;

std::string readable_time()
{
    timeval tv;
    gettimeofday(&tv, NULL);
    tv.tv_sec += 8 * 60 * 60;
    struct tm *p;
    p = gmtime(&tv.tv_sec);
    char s[80];
    strftime(s, 80, "%Y-%m-%d %H:%M:%S.", p);
    std::string st(s + std::to_string(tv.tv_usec));
    return st;
}

bool set_name(std::string_view username)
{
    for (auto &ch : username)
    {
        if (!std::isalpha(ch) && !std::isalnum(ch))
        {
            std::cout << "username should be consist of charactor or number" << std::endl;
            return false;
        }
    }
    std::string cmd("#username:");
    cmd.append(username);

    int ret = write(server_fd, cmd.c_str(), cmd.size());
    if (ret != cmd.size())
        return false;

    time_t record = time(NULL);
    while (true)
    {
        char recv[100];
        ret = read(server_fd, recv, 99);
        if (ret > 0)
        {
            if (strcmp(recv, "#username_valid") == 0)
            {
                std::cout << "set username success, enjoy your chat" << std::endl;
                return true;
            }
            else if (strcmp(recv, "#username_exist") == 0)
            {
                std::cout << "name exist, change another one" << std::endl;
                return false;
            }
            else
            {
                std::cout << "set username failed, check it's format may solve "
                             "this problem "
                          << std::endl;
                return false;
            }
        }

        if (time(nullptr) - record > 3)
        {
            std::cout << "receive message from server timeout" << std::endl;
            return false;
        }
    }
    return false;
}

void post_message(std::string_view message)
{
    char buffer[message.size() + 100];
    int mlen = snprintf(buffer, message.size() + 100, "[%s]%s:%s", readable_time().c_str(), username.c_str(), message.data());

    int total = 0;
    while (total != mlen)
    {
        int slen = write(server_fd, buffer + total, mlen - total);
        total += slen;
    }
}

void *print_other_user_message(void *)
{
    char buffer[1000];
    while (true)
    {
        int rlen = read(server_fd, buffer, 999);
        if (rlen > 0)
        {
            buffer[rlen] = 0;
            std::cout << buffer << std::endl;
        }
    }
}

void chat_client(std::string_view ip, int port)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(fd > 0);

    sockaddr_in server_addr;
    size_t server_len = sizeof server_addr;
    bzero(&server_addr, server_len);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htobe16(port);
    inet_pton(AF_INET, ip.data(), &server_addr.sin_addr);

    server_fd = connect(fd, (sockaddr *)&server_addr, server_len);
    assert(server_fd > 0);

    // register
    while (true)
    {
        std::cout << "please input your username:" << std::flush;
        std::cin >> username;
        if (set_name(username))
        {
            break;
        }
    }

    // print other's message
    pthread_t recv_thread_id;
    pthread_create(&recv_thread_id, nullptr, print_other_user_message, &fd);

    // input
    std::string content;
    while (true)
    {
        std::cout << "#" << username << ":" << std::flush;
        std::cin >> content;
        post_message(content);
    }
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout << "Usage : ./client ip port" << std::endl;
        exit(-1);
    }

    std::string ip = argv[1];
    int port = atoi(argv[2]);
    chat_client(ip, port);
}