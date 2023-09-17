#include "chatserver.hpp"
#include "chatservice.hpp"
#include "public.hpp"
#include <muduo/base/Logging.h>
#include <functional>

using namespace muduo;
using namespace muduo::net;
using namespace std::placeholders;
using namespace nlohmann;

ChatServer::ChatServer(EventLoop* loop, const InetAddress& listenAddr, const std::string& serverName)
    : server_(loop, listenAddr, serverName)
    , loop_(loop)
{
    // 给服务器注册用户连接的创建和断开回调
    server_.setConnectionCallback(
        std::bind(&ChatServer::onConnection, this, _1));

    // 给服务器注册用户读写事件回调
    server_.setMessageCallback(
        std::bind(&ChatServer::onMessage, this, _1, _2, _3));
    
    // 设置服务器线程数量 1个I/O，3个worker
    server_.setThreadNum(4);
}

void ChatServer::start()
{
    server_.start();
}

void ChatServer::onConnection(const TcpConnectionPtr& conn)
{
    LOG_INFO << "ChatServer - " << conn->peerAddress().toIpPort() << " -> "
             << conn->localAddress().toIpPort() << " is "
             << (conn->connected() ? "UP" : "DOWN");
    if (!conn->connected())
    {
        ChatService::getInstance()->clientCloseException(conn);
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time)
{
    try
    {
        muduo::string msg(buf->retrieveAllAsString());
        LOG_INFO << conn->name() << " echo " << msg.size() << " bytes, "
                 << "data received at " << time.toString();
        json js = json::parse(msg);
        int msgid = js["msgid"];

        // 如果不是注册或登录请求，先验证是否登录
        if (!(msgid == LOGIN_MSG || msgid == REG_MSG)
            && !ChatService::getInstance()->validIsLogin(conn))
        {
            json reponse;
            reponse["errno"] = 3;
            reponse["errmsg"] = "您还没有登录，请先登录！";
            conn->send(reponse.dump());
            return;
        }
        auto msgHandler = ChatService::getInstance()->getHandler(msgid);
        // 回调消息绑定好的事件处理器，来执行相应的业务处理
        msgHandler(conn, js, time);
    }
    catch (const std::exception& e)
    {
        LOG_INFO << e.what();
    }
}