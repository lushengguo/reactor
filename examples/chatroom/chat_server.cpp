#include "net/TcpServer.hpp"
#include "protocal.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <set>
#include <stdlib.h>
#include <string>
#include <string_view>
using namespace reactor;
using namespace std::placeholders;
namespace reactor
{

constexpr static char success[] = "operate success\n";
constexpr static char failed[]  = "operate failed\n";
constexpr static char help[]    = "input \"//${cmd} \" + ${message} to show "
                               "usable "
                               "command\n"
                               "for example, input \"//help \" or \"//h \" to "
                               "show "
                               "this tip\n\n"
                               "cmd shows as follows : \n\n"
                               "help(h)             :show help information\n"
                               "rooms(rs)           :show current chat rooms\n"
                               "users(us)           :show current users\n"
                               "create-room(cr)     :create a chat room and "
                               "enter it\n"
                               "enter-room(er)      :enter input room "
                               "specified by "
                               "${message}\n"
                               "chat-with-user(cwu) :redirect peer to "
                               "specific user "
                               "specified "
                               "by "
                               "${message}\n"
                               "quit(q)             :quit room or change peer "
                               "to null\n"
                               "show-members(sm)    :show members in this "
                               "chat room\n"
                               "reset-name(rn)      :reset your name to "
                               "${message}\n";

class ChatServer : noncopyable, Protocal
{
  public:
    struct UserStatus
    {
        std::string          name_;
        std::string          talking_to_;
        Status               status_;
        TcpConnectionWeakPtr conn;
    };

    typedef int                                     UserId;
    typedef std::map<std::string, std::set<UserId>> ChatRooms;
    typedef std::map<UserId, UserStatus>            OnLineUsers;

    ChatServer(EventLoop *loop, const INetAddr &addr)
      : server_(loop, addr, "ChatServer")
    {}

    // show message
    std::string chatroom_member(std::string_view room)
    {
        MutexLockGuard lock(infoGuard_);
        if (rooms_.count(room) != 1)
            return "no member";

        std::string members("users in ");
        members.append(room).append(":");
        for (auto id : rooms_.at(room))
        {
            assert(users_.count(id) == 1);
            members.append(users_.at(id).name_).append(" ");
        }
        return members;
    }

    std::string all_chatroom()
    {
        MutexLockGuard lock(infoGuard_);
        std::string    rooms("all chatrooms:");
        for (auto room : rooms_) rooms.append(room.first).append("");
        return rooms;
    }

    std::string online_users()
    {
        MutexLockGuard lock(infoGuard_);
        std::string    users("all online users:");
        for (auto user : users_) users.append(user.second.name_).append(" ");
        return users;
    }

    bool set_name(UserId id, std::string_view new_name)
    {
        if (new_name.empty() || rooms_.count(new_name) == 1)
            return false;

        users_.at(id).name_ = new_name;
        return true;
    }

    void new_user(UserId id, TcpConnectionPtr conn)
    {
        users_.insert(
          std::make_pair(id, UserStatus{"", "", kNotSetUserName, conn}));
    }

    bool new_chatroom(std::string_view name)
    {
        MutexLockGuard lock(infoGuard_);
        if (rooms_.count(name) == 1)
            return false;
        rooms_.insert(std::make_pair<name, std::set<UserId>>);
        return true;
    }

    // name could be username or chatroom name
    bool checkout(UserId id, std::string_view name)
    {
        MutexLockGuard lock(infoGuard_);
        // is name refers to a room name ?
        if (rooms_.count(name) == 1)
        {
            users_.at(id).talking_to_ = name;
            return true;
        }

        // is name refers to a user name ?
        auto iter = std::find_if(users_.begin(),
          users_.end(),
          [&](const UserStatus &state) { return state.name_ == name; });

        if (iter == users_.end())
            return false;

        users_.at(id).talking_to_ = name;
        return true;
    }

    bool quit(UserId id) { users_.at(id).talking_to_ = ""; }

    void post_message_to_id(UserId id, std::string_view message) {}

    // id是发送方的id
    void post_message(UserId id, std::string_view message)
    {
        MutexLockGuard   lock(infoGuard_);
        std::string_view talking_to(users_.at(id).talking_to_);
        if (talking_to == "")
            return;

        // is talking_to refers to a room name ?
        if (rooms_.count(talking_to) == 1)
        {
            for (auto id : rooms_.at(talking_to))
                post_message_to_id(id, message);
            return;
        }

        // is talking_to refers to a user name ?
        auto iter = std::find_if(users_.begin(),
          users_.end(),
          [&](const UserStatus &state) { return state.name_ == talking_to; });
        assert(iter != users_.end());
        post_message_to_id(iter->first, message);
    }

    std::string_view operate_tip(bool b) { return b ? success : failed; }

    void onConnection(TcpConnectionPtr conn)
    {
        new_user(conn->fd(), conn);
        conn->send(help);
    }

    void onMessage(TcpConnectionPtr conn, Buffer &buffer)
    {
        std::string message;
        bool        r  = true; //默认操作成功
        UserId      id = conn->fd();
        Cmd         cmd =
          server_parse_message(buffer.string(buffer.readable_bytes()), message);
        switch (cmd)
        {
        case kSetName: r = set_name(id, message); break;
        case kRequestHelp: conn->send(help); break;
        case kCreateChatRoom:
            r = new_chatroom(message);
            if (r)
                checkout(id, message);
            break;
        case kViewChatRoomMember: conn->send(chatroom_member(message)); break;
        case kViewAllChatRoom: conn->send(all_chatroom()); break;
        case kViewOnlineUser: conn->send(online_users()); break;
        case kChatWithUser: r = checkout(id, message); break;
        case kQuitChat: r = quit(id); break;
        case kMessage: post_message(id, message); break;
        case kError: r = false; break;

        default: break;
        }
        conn->send(operate_tip(r));
    }

    void start()
    {
        server_.set_onConnectionCallback(std::bind(onConnection, this, _1));
        server_.set_onMessageCallback(std::bind(onMessage, this, _1, _2));
        server_.start();
    }

  private:
    TcpServer   server_;
    Mutex       infoGuard_;
    ChatRooms   rooms_;
    OnLineUsers users_;
};

} // namespace reactor

int main(int argc, char **argv)
{
    if (argc <= 4)
    {
        std::cerr << "Usage: ./char_server ip port threadnum" << std::endl;
        return -1;
    }

    INetAddr   addr(argv[1], atoi(argv[2]));
    int        threadnum = atoi(argv[3]);
    EventLoop  loop;
    ChatServer server(&loop, addr);
    server.start();
    loop.loop();
}