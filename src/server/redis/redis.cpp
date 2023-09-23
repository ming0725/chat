#include "redis.hpp"
#include <thread>
#include <muduo/base/Logging.h>

using namespace std;

Redis::~Redis()
{
    if (!publishContext_)
    {
        redisFree(publishContext_);
    }
    if (!subscribeContext_)
    {
        redisFree(subscribeContext_);
    }
}

bool Redis::connect(string ip, int port)
{
    publishContext_ = redisConnect(ip.c_str(), port);
    subscribeContext_ = redisConnect(ip.c_str(), port);
    if (!publishContext_ || !subscribeContext_)
    {
        LOG_INFO << "redis connect failed!";
        return false;
    }
    thread t([this](){ observerChannelMessage(); });
    t.detach();
    return true;
}

bool Redis::publish(int channel, string message)
{
    redisReply* reply = (redisReply*)redisCommand(publishContext_, "PUBLISH %d %s", channel, message.c_str());
    if (reply == nullptr)
    {
        LOG_INFO << "publish command failed!";
        return false;
    }
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel)
{
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道，不接收通道消息
    // 通道消息的接收专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送命令，不阻塞接收redis server响应消息，否则和notifyMsg线程抢占响应资源
    if (REDIS_ERR == redisAppendCommand(subscribeContext_, "SUBSCRIBE %d", channel))
    {
        LOG_INFO << "subscribe command failed!";
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(subscribeContext_, &done))
        {
            LOG_INFO << "subscribe command failed!";
            return false;
        }
    }
    // redisGetReply
    return true;
}

bool Redis::unsubscribe(int channel)
{
    if (REDIS_ERR == redisAppendCommand(subscribeContext_, "UNSUBSCRIBE %d", channel))
    {
        LOG_INFO << "unsubscribe command failed!";
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区，直到缓冲区数据发送完毕（done被置为1）
    int done = 0;
    while (!done)
    {
        if (REDIS_ERR == redisBufferWrite(subscribeContext_, &done))
        {
            LOG_INFO << "unsubscribe command failed!";
            return false;
        }
    }
    return true;
}

void Redis::observerChannelMessage()
{
    redisReply *reply = nullptr;
    while (REDIS_OK == redisGetReply(subscribeContext_, (void**)&reply))
    {
        // 订阅收到的消息是一个带三元素的数组
        if (reply != nullptr 
            && reply->element[2] != nullptr 
            && reply->element[2]->str != nullptr)
        {
            // 给业务层上报通道上发生的消息
            notifyMsgHandler_(atoi(reply->element[1]->str) , reply->element[2]->str);
        }
        freeReplyObject(reply);
    }
}

void Redis::initNotifyHandler(function<void(int, string)> func)
{
    notifyMsgHandler_ = func;
}
