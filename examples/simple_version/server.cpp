#include <assert.h>
#include <endian.h>
#include <errno.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <netinet/in.h>
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
#include <vector>
#define LISTEN_QUEUE 100
static std::map<int, std::string> namemap;

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

const char *username(int fd)
{
    if (namemap.count(fd) == 0 || namemap[fd].empty())
        return nullptr;
    else
        return namemap.at(fd).c_str();
}

bool name_exist(std::string_view name)
{
    for (auto &ele : namemap)
    {
        if (ele.second == name)
        {
            return true;
        }
    }
    return false;
}

bool user_registed(int fd) { return namemap.count(fd) == 1; }

void user_register(int fd, std::string_view message)
{
    if (user_registed(fd))
        return;

    if (message.size() < strlen("#username:"))
        return;

    char *ret[] = {"#username_valid", "#username_exist", "#username_invalid"};
    if (message.substr(0, strlen("#username:")) != "#username:")
    {
        std::string name = message.substr(strlen("#username:")).data();
        if (name.empty())
        {
            write(fd, ret[2], strlen(ret[2]));
            std::cout << "user register failed, name invalid" << std::endl;
        }
        else if (name_exist(name))
        {
            write(fd, ret[1], strlen(ret[1]));
            std::cout << "user register failed, name exist" << std::endl;
        }
        else
        {
            write(fd, ret[0], strlen(ret[0]));
            std::cout << "user register success, name " << name << std::endl;
        }
    }
}

void send_to_all_user_except(int fd, std::string_view message)
{
    if (message.empty())
        return;

    assert(user_registed(fd));
    for (auto &user : namemap)
    {
        if (fd != user.first)
            write(user.first, message.data(), message.size());
    }
}

void chat_server(int port)
{
    std::vector<epoll_event> active_events(10);

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    assert(server_fd > 0);

    sockaddr_in saddr;
    bzero(&saddr, sizeof saddr);
    saddr.sin_family = AF_INET;
    saddr.sin_port = htobe16(port);
    saddr.sin_addr.s_addr = htobe64(INADDR_ANY);
    int ret = bind(server_fd, (const sockaddr *)&saddr, sizeof saddr);
    assert(ret == 0);

    ret = listen(server_fd, 100);
    assert(ret == 0);

    int epollfd = epoll_create(100);
    assert(epollfd > 0);
    epoll_event event;
    event.events |= EPOLLIN;
    event.data.fd = server_fd;
    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, server_fd, &event);
    assert(ret == 0);
    while (true)
    {
        int active_num = epoll_wait(epollfd, active_events.data(), active_events.size(), 10);
        if (active_num > 0)
        {
            for (int i = 0; i < active_num; ++i)
            {
                int active_fd = active_events[i].data.fd;
                // new connection
                if (active_fd == server_fd)
                {
                    sockaddr_in peer_addr;
                    socklen_t peer_len = sizeof peer_addr;
                    bzero(&peer_addr, peer_len);
                    int peer_fd = accept(server_fd, (sockaddr *)&peer_addr, &peer_len);
                    printf("new connection\n");
                    epoll_event peer_event;
                    peer_event.data.fd = peer_fd;
                    peer_event.events |= EPOLLIN;
                    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, peer_fd, &peer_event);
                    assert(ret == 0);
                }
                else // user's message
                {
                    char message[1000];
                    int rlen = read(active_fd, message, 999);
                    message[rlen] = 0;
                    if (rlen > 0)
                    {
                        if (user_registed(active_fd))
                        {
                            send_to_all_user_except(active_fd, message);
                        }
                        else
                        {
                            user_register(active_fd, message);
                        }
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    assert(argc == 2);
    int port = atoi(argv[1]);
    chat_server(port);
}