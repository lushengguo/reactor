#pragma once
#ifndef REACTOR_EXAMPLE_PROTOCAL_HPP
#define REACTOR_EXAMPLE_PROTOCAL_HPP

#include <string>
#include <string_view>
namespace reactor
{
// Protocal : #cmd#message
class Protocal
{
  public:
    enum Cmd
    {
        kSetName,
        kRequestHelp,
        //创建的用户不拥有这个聊天室 他只是定一个topic然后大家进入讨论
        //当创建聊天室的用户退出的时候聊天室不会立即删除
        //当最后一个用户退出聊天室的时候聊天室将消失
        kCreateChatRoom,
        //查看用户
        kViewChatRoomMember,
        kViewAllChatRoom,
        kViewOnlineUser,
        //切换聊天对象 可以是chatroom也可以是用户 甚至是null
        kEnterChatRoom,
        kChatWithUser,
        kQuitChat,
        //
        kMessage,
        kError
    };

    enum Status
    {
        kNotSetUserName,
        k
    };

    //简单点 用户输入什么client就发给server什么
    // server返回什么client端都直接打印
    //这样只需要处理server端的逻辑就可以了

    //缺点是费流量 占通讯资源 网络状态不好时用户不会收到提示信息

    Cmd server_parse_message(const std::string &input, std::string &message);
};

} // namespace reactor

#endif