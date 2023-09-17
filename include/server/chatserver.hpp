#ifndef CHATSERVER_HPP
#define CHATSERVER_HPP

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>

// 使用muduo开发服务器
class ChatServer
{
public:

    ChatServer(muduo::net::EventLoop* loop,
               const muduo::net::InetAddress& listenAddr,
               const std::string& serverName);

    void start();

private:

    void onConnection(const muduo::net::TcpConnectionPtr& conn);

    void onMessage(const muduo::net::TcpConnectionPtr& conn,
                   muduo::net::Buffer* buf,
                   muduo::Timestamp time);

private:
    muduo::net::TcpServer server_;
    muduo::net::EventLoop* loop_;
};


#endif // CHATSERVER_HPP