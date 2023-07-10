#include "net/TcpServer.hpp"
#include "protocol.hpp"
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
constexpr static char failed[] = "operate failed\n";
constexpr static char help[] = "input \"//${cmd} \" + ${message} to show "
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
        std::string name_;
        std::string talking_to_;
        Status status_;
        TcpConnectionWeakPtr conn;
    };

    typedef int UserId;
    typedef std::map<std::string, std::set<UserId>> ChatRooms;
    typedef std::map<UserId, UserStatus> OnLineUsers;

    ChatServer(EventLoop *loop, const INetAddr &addr) : server_(loop, addr, "ChatServer") {}

    // show message
    std::string chatroom_member(std::string_view room)
    {
        if (rooms_.count(std::string(room)) != 1)
            return "no member";

        std::string members("users in ");
        members.append(room).append(":");
        for (auto id : rooms_.at(std::string(room)))
        {
            assert(users_.count(id) == 1);
            members.append(users_.at(id).name_).append(" ");
        }
        return members;
    }

    std::string all_chatroom()
    {
        std::string rooms("all chatrooms:");
        for (auto &room : rooms_)
            rooms.append(room.first).append(" ");
        return rooms;
    }

    std::string online_users()
    {
        std::string users("all online users:");
        for (auto user : users_)
            users.append(user.second.name_).append(" ");
        return users;
    }

    bool set_name(UserId id, std::string_view new_name)
    {
        if (new_name.empty() || rooms_.count(std::string(new_name)) == 1)
            return false;

        users_.at(id).name_ = new_name;
        return true;
    }

    void new_user(UserId id, TcpConnectionPtr conn) { users_.insert(std::make_pair(id, UserStatus{"", "", kNotSetUserName, conn})); }

    bool new_chatroom(const std::string &name)
    {
        if (rooms_.count(name) == 1)
            return false;
        rooms_.insert(std::make_pair(name, std::set<UserId>()));
        return true;
    }

    bool is_chatroom_name(std::string_view name) { return rooms_.count(name.data()); }

    // return val>0 if is user name
    UserId is_user_name(std::string_view name)
    {
        for (const auto &user : users_)
        {
            if (user.second.name_ == name)
                return user.first;
        }
        return 0;
    }

    bool checkout(UserId id, const std::string &name)
    {
        if (is_chatroom_name(name))
        {
            users_.at(id).talking_to_ = name;
            rooms_.at(name).insert(id);
            return true;
        }

        if (UserId uid; (uid = is_user_name(name)) > 0)
        {
            users_.at(id).talking_to_ = name;
            return true;
        }

        return false;
    }

    void quit(UserId id) { users_.at(id).talking_to_ = ""; }

    void collect_user(UserId id)
    {
        if (users_.count(id == 1))
            users_.erase(id);
    }

    void collect_room(const std::string &room)
    {
        if (rooms_.count(room) == 1)
            rooms_.erase(room);
    }

    void post_message_to_id(UserId id, std::string_view message)
    {
        if (users_.count(id) == 1)
        {
            auto conn = users_.at(id).conn.lock();
            if (conn)
            {
                conn->send(message);
            }
            else
            {
                std::string talking_to(users_.at(id).talking_to_);
                if (is_chatroom_name(talking_to) || rooms_.at(talking_to).size() == 1)
                    collect_room(talking_to);
                collect_user(id);
            }
        }
    }

    void post_message_to_room(UserId from, std::string_view room_name, std::string_view message)
    {
        log_info("redirect message from to room=%s", room_name.data());
        if (rooms_.count(room_name.data()) == 0)
            return;

        for (auto uid : rooms_.at(std::string(room_name)))
        {
            if (from != uid)
                post_message_to_id(uid, message);
        }
    }

    std::string package_message(UserId id, std::string_view message, MilliTimestamp receive_timestamp,
                                const char *room_name = nullptr)
    {
        std::string m("[");
        m.append(fmt_timestamp(receive_timestamp / 1000000)).append(",from user ").append(users_.at(id).name_);
        if (room_name)
            m.append(" in room ").append(room_name);
        m.append("]").append(message);
        return m;
    }

    // id是发送方的id
    void post_message(UserId id, std::string_view message, MilliTimestamp receive_timstamp)
    {
        std::string_view talking_to(users_.at(id).talking_to_);
        if (talking_to == "")
            return;

        if (is_chatroom_name(talking_to))
        {
            std::string m = package_message(id, message, receive_timstamp, talking_to.data());
            post_message_to_room(id, talking_to, m);
        }

        if (UserId uid; (uid = is_user_name(talking_to)) > 0)
        {
            std::string m = package_message(id, message, receive_timstamp);
            post_message_to_id(uid, m);
        }
    }

    std::string show_position(UserId id)
    {
        std::string talking_to(users_.at(id).talking_to_);
        if (is_chatroom_name(talking_to))
            return std::string("you are in room " + talking_to);
        else if (talking_to.empty())
            return "you are talking to nobody";
        else
            return std::string("you are talking to " + talking_to);
    }

    void onConnection(TcpConnectionPtr conn)
    {
        new_user(conn->fd(), conn);
        conn->send(help);
    }

    void onMessage(TcpConnectionPtr conn, Buffer &buffer, MilliTimestamp receive_timestamp)
    {
        std::string message;
        bool r = true; //默认操作成功
        UserId id = conn->fd();
        Cmd cmd = server_parse_message(std::string(buffer.read_all_as_string()), message);
        buffer.retrive_all();

        log_debug("user name[%s] input cmd[%s],message[%s]", users_.at(id).name_.c_str(), cmd_to_string(cmd).c_str(), message.c_str());

        if (cmd != kSetName && users_.at(id).name_.empty())
        {
            conn->send("plz set your name first , \"//reset-name [yourname]\"\n");
            return;
        }

        switch (cmd)
        {
        case kSetName:
            r = set_name(id, message);
            break;
        case kRequestHelp:
            conn->send(help);
            break;
        case kCreateChatRoom:
            r = new_chatroom(message);
            if (r)
                checkout(id, message);
            break;
        case kViewChatRoomMember:
            conn->send(chatroom_member(message));
            break;
        case kViewAllChatRoom:
            conn->send(all_chatroom());
            break;
        case kViewOnlineUser:
            conn->send(online_users());
            break;
        case kChatWithUser:
            r = checkout(id, message);
            break;
        case kEnterChatRoom:
            r = checkout(id, message);
            break;
        case kQuitChat:
            quit(id);
            break;
        case kMessage:
            post_message(id, message, receive_timestamp);
            break;
        case kPosition:
            conn->send(show_position(id));
            break;
        case kError:
            r = false;
            break;

        default:
            r = false;
            break;
        }

        if (!r)
            conn->send(failed);
    }

    void onDisconnect() {}

    void start()
    {
        server_.set_onConnectionCallback(std::bind(&ChatServer::onConnection, this, _1));
        server_.set_onMessageCallback(std::bind(&ChatServer::onMessage, this, _1, _2, _3));
        server_.start();
    }

  private:
    TcpServer server_;
    ChatRooms rooms_;
    OnLineUsers users_;
};

} // namespace reactor

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: ./char_server port" << std::endl;
        return -1;
    }

    INetAddr addr("", atoi(argv[1]));
    EventLoop loop;
    ChatServer server(&loop, addr);
    server.start();
    loop.loop();
}