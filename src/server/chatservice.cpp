#include "chatservice.hpp"
#include "public.hpp"

#include <functional>
#include <muduo/base/Logging.h>

using namespace std;
using namespace std::placeholders;
using namespace nlohmann;
using namespace muduo;
using namespace muduo::net;

ChatService* ChatService::getInstance()
{
    static ChatService service;
    return &service;
}

ChatService::ChatService()
{
    if (redis_.connect("127.0.0.1", 6379))
    {
        redis_.initNotifyHandler(std::bind(&ChatService::handleRedisSubscribeMsg, this, _1, _2));
    }
    msgHandlerMap_.emplace(LOGIN_MSG,        std::bind(&ChatService::login,       this, _1, _2, _3));
    msgHandlerMap_.emplace(LOGINOUT_MSG,     std::bind(&ChatService::logout,      this, _1, _2, _3));
    msgHandlerMap_.emplace(REG_MSG,          std::bind(&ChatService::reg,         this, _1, _2, _3));
    msgHandlerMap_.emplace(ONE_CHAT_MSG,     std::bind(&ChatService::oneChat,     this, _1, _2, _3));
    msgHandlerMap_.emplace(ADD_FRIEND_MSG,   std::bind(&ChatService::addFriend,   this, _1, _2, _3));
    msgHandlerMap_.emplace(CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, _1, _2, _3));
    msgHandlerMap_.emplace(ADD_GROUP_MSG,    std::bind(&ChatService::addGroup,    this, _1, _2, _3));
    msgHandlerMap_.emplace(GROUP_CHAT_MSG,   std::bind(&ChatService::groupChat,   this, _1, _2, _3));
    msgHandlerMap_.emplace(REFRESH_INFO_MSG, std::bind(&ChatService::refreshInfo, this, _1, _2, _3));
}

void ChatService::login(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    long id = js["id"];
    std::string pwd = js["password"];
    User user = userModel_.query(id);
    json response;
    if (!validIsLogin(conn))
    {
        if (user.getPassword() == pwd && user.getId() == id)
        {
            if (userModel_.query(id).getState() == "offline")
            {
                userConnMapMtx_.lock();
                userConnMap_.insert({id, conn});
                userConnMapMtx_.unlock();
                userModel_.updateState(user);
                redis_.subscribe(id);
                response["errno"] = 0;
                initUserInfo(id, response);
            }
            else 
            {
                response["errno"] = 2;
                response["errmsg"] = "该账号已登录";
            }
        }
        else
        {
            response["errno"] = 1;
            response["errmsg"] = "密码错误！";
        }
    }
    else
    {
        response["errno"] = 2;
        response["errmsg"] = "您已登录一个账号，不能重复登录";
    }
    response["msgid"] = REG_MSG_ACK;
    conn->send(response.dump());
}

void ChatService::logout(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    clientCloseException(conn);
}

void ChatService::reg(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    User user;
    json response;
    user.setName(js["name"]);
    user.setPassword(js["password"]);
    user.setState("offline");
    if (userModel_.insert(user))
    {
        response["msgid"] = REG_MSG_ACK;
        response["id"] = user.getId();
        response["errno"] = 0;
    }
    else
    {
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
    }
    conn->send(response.dump());
}

ChatService::MsgHandler ChatService::getHandler(int msgid)
{
    if (msgHandlerMap_.count(msgid) == 0)
    {
        return [=](const TcpConnectionPtr&, json&, Timestamp)
        { LOG_ERROR << "msgid: " << msgid << " can not find handler"; };
    }
    return msgHandlerMap_[msgid];
}

void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
    lock_guard<mutex> lock(userConnMapMtx_);
    if (userConnMap_.right.count(conn) != 0)
    {
        User user;
        user.setId(userConnMap_.right.at(conn));
        userModel_.resetState(user);
        userConnMap_.right.erase(conn);
        redis_.unsubscribe(user.getId());
    }
}

void ChatService::serverCloseException()
{
    lock_guard<mutex> lock(userConnMapMtx_);
    for (auto& [id, conn]: userConnMap_)
    {
        User user;
        user.setId(id);
        userModel_.resetState(user);
        redis_.unsubscribe(id);
    }
}

void ChatService::oneChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    long fromId = getConectedUserId(conn);
    long toId = js["to"];
    json response;
    if (toId == fromId)
    {
        response["errno"] = 2;
        response["errmsg"] = "不能给自己发送消息";
        response["msgid"] = ONE_CHAT_MSG;
        conn->send(response.dump());
    }
    else if (userModel_.query(toId).getId() == 0)
    {
        response["errno"] = 2;
        response["errmsg"] = "该用户不存在";
        response["msgid"] = ONE_CHAT_MSG;
        conn->send(response.dump());
    }
    else
    {
        std::string msg = js["msg"];
        js["time"] = time.toFormattedString();
        js["from"] = fromId;
        lock_guard<mutex> lock(userConnMapMtx_);
        if (userConnMap_.left.count(toId) == 1)
        {
            auto& toConn = userConnMap_.left.at(toId);
            toConn->send(js.dump());
        }
        else if (userModel_.query(toId).getState() == "online")
        {
            redis_.publish(toId, js.dump());
        }
        else 
        {
            offlineMsgModel_.insert(toId, js.dump());
        }
        response["errno"] = 0;
        response["msgid"] = ONE_CHAT_MSG;
        conn->send(response.dump());
    }
}


void ChatService::addFriend(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    long userId = getConectedUserId(conn);
    long friendId = js["friendid"];
    json response;
    if (userModel_.query(friendId).getId() == 0)
    {
        response["errno"] = 1;
        response["errmsg"] = "该用户不存在";
        response["msgid"] = ADD_FRIEND_MSG;
    }
    else
    {
        friendModel_.insert(userId, friendId);
        response["errno"] = 0;
        response["msgid"] = ADD_FRIEND_MSG;
    }
    conn->send(response.dump());
}

void ChatService::createGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    long sendId = getConectedUserId(conn);
    json response;
    Group group;
    group.setName(js["groupname"]);
    group.setDesc(js["groupdesc"]);
    if (groupModel_.createGroup(group))
    {
        response["errno"] = 0;
        response["groupid"] = group.getId();
        response["msgid"] = CREATE_GROUP_MSG;
        groupModel_.addGroup(sendId, group.getId(), "creator");
    }
    else 
    {
        response["errno"] = 1;
        response["errmsg"] = "创建群聊失败";
        response["msgid"] = CREATE_GROUP_MSG;
    }
    conn->send(response.dump());
}

void ChatService::addGroup(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    long sendId = getConectedUserId(conn);
    long groupId = js["groupid"];
    json response;
    if (groupModel_.queryGroupInfo(groupId).getId() == 0)
    {
        response["errno"] = 1;
        response["errmsg"] = "群组不存在";
        response["msgid"] = ADD_GROUP_MSG;
    }
    else 
    {
        groupModel_.addGroup(sendId, groupId, "normal");
        response["errno"] = 0;
        response["msgid"] = ADD_GROUP_MSG;
    }
    conn->send(response.dump());
}

void ChatService::groupChat(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    json response;
    response["msgid"] = GROUP_CHAT_MSG;
    try
    {
        long sendId = getConectedUserId(conn);
        long groupId = js["groupid"];
        vector<long> groupUsers = groupModel_.queryGroupUsers(sendId, groupId);
        js["from"] = sendId;
        js["time"] = time.toFormattedString();
        lock_guard<mutex> lock(userConnMapMtx_);
        for (auto id: groupUsers)
        {
            if (userConnMap_.left.count(id))
            {
                userConnMap_.left.at(id)->send(js.dump());
            }
            else if (userModel_.query(id).getState() == "online")
            {
                redis_.publish(id, js.dump());
            }
            else
            {
                offlineMsgModel_.insert(sendId, js.dump());
            }
        }
        response["errno"] = 0;
        conn->send(response.dump());
    }
    catch (const std::exception& e)
    {
        response["errno"] = 1;
        conn->send(response.dump());
        LOG_INFO << e.what();
    }
}

void ChatService::handleRedisSubscribeMsg(int userId, string msg)
{
    json js = json::parse(msg);
    long sendId = js["from"];
    lock_guard<mutex> lock(userConnMapMtx_);
    if (userConnMap_.left.count(userId) != 0)
    {
        userConnMap_.left.at(userId)->send(msg);
    }
    else 
    {
        offlineMsgModel_.insert(userId, msg);
    }
}

void ChatService::initUserInfo(long id, json& js)
{
    vector<string> friends{};
    vector<string> groups{};
    vector<User>  userVec  = friendModel_.query(id);
    vector<Group> groupVec = groupModel_.queryGroups(id);
    for (auto& user: userVec)
    {
        json js;
        js["id"]    = user.getId();
        js["name"]  = user.getName();
        js["state"] = user.getState();
        friends.emplace_back(js.dump());
    }
    for (auto& group: groupVec)
    {
        json js;
        js["id"]   = group.getId();
        js["name"] = group.getName();
        js["desc"] = group.getDesc();
        groups.emplace_back(js.dump());
    }
    js["friends"]    = friends;
    js["groups"]     = groups;
    js["offlinemsg"] = offlineMsgModel_.query(id);
    User user  = userModel_.query(id);
    js["id"]   = user.getId();
    js["name"] = user.getName();
    offlineMsgModel_.remove(id);
}

void ChatService::refreshInfo(const TcpConnectionPtr& conn, json& js, Timestamp time)
{
    try
    {
        long sendId = getConectedUserId(conn);
        initUserInfo(sendId, js);
        js["errno"] = 0;
        conn->send(js.dump());
    }
    catch (const exception& e)
    {
        js["errno"] = 1;
        LOG_INFO << e.what();
        conn->send(js.dump());
    }
}

long ChatService::getConectedUserId(const TcpConnectionPtr& conn)
{
    lock_guard<mutex> lock(userConnMapMtx_);
    return userConnMap_.right.at(conn);
}

bool ChatService::validIsLogin(const TcpConnectionPtr& conn)
{
    lock_guard<mutex> lock(userConnMapMtx_);
    return (userConnMap_.right.count(conn) == 1);
}