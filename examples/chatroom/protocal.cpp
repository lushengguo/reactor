#include "protocal.hpp"
#include <regex>

namespace reactor
{
Protocal::Cmd Protocal::server_parse_message(
  const std::string &raw_data, std::string &extra_message)
{
    std::regex  without_message("//([a-z]+)\\s*");
    std::regex  with_message("//([a-z]+)\\s+((\\w+))");
    std::smatch result;
    if (std::regex_match(raw_data, result, without_message))
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
        else if (cmd == "position" || cmd == "p")
            return kPosition;
        else
            return kError;
    }
    else if (std::regex_match(raw_data, result, with_message))
    {
        std::string_view cmd(result[1].str());
        std::string_view pm(result[2].str());
        if (cmd == "create-room" || cmd == "cr")
        {
            extra_message = pm;
            return kCreateChatRoom;
        }
        else if (cmd == "enter-room" || cmd == "er")
        {
            extra_message = pm;
            return kEnterChatRoom;
        }
        else if (cmd == "char-with-user" || cmd == "cwu")
        {
            extra_message = pm;
            return kChatWithUser;
        }
        else if (cmd == "reset-name" || cmd == "rn")
        {
            extra_message = pm;
            return kSetName;
        }
        else
        {
            return kError;
        }
    }

    extra_message = raw_data;
    return kMessage;
}

std::string Protocal::cmd_to_string(Cmd cmd)
{
    switch (cmd)
    {
    case kSetName: return "SetName";
    case kRequestHelp: return "RequestHelp";
    case kCreateChatRoom: return "CreateChatRoom";
    case kViewChatRoomMember: return "ViewChatRoomMember";
    case kViewAllChatRoom: return "ViewAllChatRoom";
    case kViewOnlineUser: return "ViewOnlineUser";
    case kEnterChatRoom: return "EnterChatRoom";
    case kChatWithUser: return "ChatWithUser";
    case kQuitChat: return "QuitChat";
    case kMessage: return "Message";
    case kPosition: return "Position";
    case kError: return "Error";
    default: return "InvalidCmd";
    }
}
} // namespace reactor