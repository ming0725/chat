#include "chatserver.hpp"
#include "chatservice.hpp"
#include <signal.h>

void resetHandler(int)
{
    ChatService::getInstance()->serverCloseException();
    exit(1);
}

int main(int argc, char* argv[])
{
    signal(SIGINT, &resetHandler);
    char* ip = argv[1];
    uint16_t port = atoi(argv[2]);
    muduo::net::EventLoop loop;
    muduo::net::InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");
    server.start();
    loop.loop();
    return 0;
}
