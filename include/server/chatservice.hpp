#ifndef CHATSERVICE_HPP
#define CHATSERVICE_HPP

#include <mutex>
#include <functional>
#include <unordered_map>
#include <muduo/net/TcpConnection.h>
#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <json.hpp>

#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include "redis.hpp"

class ChatService
{
    using MsgHandler = std::function<void(const muduo::net::TcpConnectionPtr&, 
                                          nlohmann::json&,
                                          muduo::Timestamp)>;
public:

    // 获取单例对象实例
    static ChatService* getInstance();

    // 用户登录业务
    void login(const muduo::net::TcpConnectionPtr& conn, 
               nlohmann::json& js,
               muduo::Timestamp time);
               
    // 用户注销
    void logout(const muduo::net::TcpConnectionPtr& conn, 
                nlohmann::json& js,
                muduo::Timestamp time);

    // 用户注册业务
    void reg(const muduo::net::TcpConnectionPtr& conn, 
             nlohmann::json& js,
             muduo::Timestamp time);

    // 一对一聊天
    void oneChat(const muduo::net::TcpConnectionPtr& conn, 
                 nlohmann::json& js,
                 muduo::Timestamp time);

    // 添加好友
    void addFriend(const muduo::net::TcpConnectionPtr& conn, 
                   nlohmann::json& js,
                   muduo::Timestamp time);

    // 创建群组
    void createGroup(const muduo::net::TcpConnectionPtr& conn, 
                     nlohmann::json& js,
                     muduo::Timestamp time);    

    // 加入群组
    void addGroup(const muduo::net::TcpConnectionPtr& conn, 
                  nlohmann::json& js,
                  muduo::Timestamp time);

    // 群聊
    void groupChat(const muduo::net::TcpConnectionPtr& conn, 
                   nlohmann::json& js,
                   muduo::Timestamp time);

    void refreshInfo(const muduo::net::TcpConnectionPtr& conn, 
                     nlohmann::json& js,
                     muduo::Timestamp time);
;

    // 获取对应的控制器
    MsgHandler getHandler(int msgid);

    // 处理客户端异常退出
    void clientCloseException(const muduo::net::TcpConnectionPtr& conn);

    // 处理服务器异常退出
    void serverCloseException();

    // 检测连接是否登录
    bool validIsLogin(const muduo::net::TcpConnectionPtr& conn);

private:
    ChatService(); 

    void initUserInfo(long id, nlohmann::json& js);

    long getConectedUserId(const muduo::net::TcpConnectionPtr& conn);

    // Redis订阅回调
    void handleRedisSubscribeMsg(long userId, std::string msg);

private:
    std::mutex userConnMapMtx_;
    std::unordered_map<int, MsgHandler> msgHandlerMap_;
    boost::bimaps::bimap<boost::bimaps::unordered_set_of<long>, 
                         boost::bimaps::unordered_set_of<muduo::net::TcpConnectionPtr>> userConnMap_;
private:
    UserModel           userModel_;
    FriendModel         friendModel_;
    GroupModel          groupModel_;
    OfflineMessageModel offlineMsgModel_;
    Redis               redis_;
};


#endif //CHATSERVICE_HPP