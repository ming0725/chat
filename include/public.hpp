#ifndef PUBLIC_HPP
#define PUBLIC_HPP

enum EnMsgType
{
    LOGIN_MSG = 1,      // 1登录消息
    LOGIN_MSG_ACK,      // 2登录响应消息
    LOGINOUT_MSG,       // 3注销消息
    REG_MSG,            // 4注册消息
    REG_MSG_ACK,        // 5注册响应消息
    ONE_CHAT_MSG,       // 6聊天消息
    ADD_FRIEND_MSG,     // 7添加好友消息
    CREATE_GROUP_MSG,   // 8创建群组
    ADD_GROUP_MSG,      // 9加入群组
    GROUP_CHAT_MSG,     // 10群聊天
    REFRESH_INFO_MSG,   // 11刷新个人信息
};

#endif // PUBLIC_HPP