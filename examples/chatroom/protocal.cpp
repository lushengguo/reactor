#include "protocal.hpp"
#include <regex>

namespace reactor
{
std::string Protocal::help_message() const
{
    return "input \"//${cmd} \" + ${message} to show usable command\n"
           "for example, input \"//help \" or \"//h \" to show this tip\n\n"
           "cmd shows as follows : \n\n"
           "help(h)             :show help information\n"
           "rooms(rs)           :show current chat rooms\n"
           "users(us)           :show current users\n"
           "create-room(cr)     :create a chat room and enter it\n"
           "enter-room(er)      :enter input room specified by ${message}\n"
           "chat-with-user(cwu) :redirect peer to specific user specified by "
           "${message}\n"
           "quit(q)             :quit room or change peer to null\n"
           "show-members(sm)    :show members in this chat room\n"
           "reset-name(rn)      :reset your name to ${message}\n";
}

Protocal::Cmd Protocal::server_parse_message(
  const std::string &input, std::string &message)
{
    std::regex  without_message("//([a-z]+)\\s*");
    std::regex  with_message("//([a-z]+)\\s+((\\w+))");
    std::smatch result;
    if (std::regex_match(input, result, without_message))
    {
        //这些指令是不需要输入message变量的
        std::string_view cmd(result[1].str());
        if (cmd == "help" || cmd == "h")
            return kRequestHelp;
        else if (cmd == "rooms" || cmd == "rs")
            return kViewAllChatRoom;
        else if (cmd == "users" || cmd == "us")
            return kViewOnlineUser;
        else if (cmd == "quit" || cmd == "q")
            return kQuitChat;
        else if (cmd == "show-members" || cmd == "sm")
            return kViewChatRoomMember;
        else
            return kError;
    }
    else if (std::regex_match(input, result, with_message))
    {
        std::string_view cmd(result[1].str());
        std::string_view pm(result[2].str());
        if (cmd == "create-room" || cmd == "cr")
        {
            message = pm;
            return kCreateChatRoom;
        }
        else if (cmd == "enter-room" || cmd == "er")
        {
            message = pm;
            return kEnterChatRoom;
        }
        else if (cmd == "char-with-user" || cmd == "cwu")
        {
            message = pm;
            return kChatWithUser;
        }
        else if (cmd == "reset-name" || cmd == "rn")
        {
            message = pm;
            return kSetName;
        }
        else
        {
            return kError;
        }
    }

    message = input;
    return kMessage;
}

std::string Protocal::tips(Cmd cmd, std::string_view message)
{
    switch (cmd)
    {
    case /* constant-expression */:
        /* code */
        break;

    default: break;
    }
}

} // namespace reactor