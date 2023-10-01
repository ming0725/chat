#ifndef REDIS_HPP
#define REDIS_HPP

#include <hiredis/hiredis.h>
#include <functional>
#include <string>

class Redis
{
public:
    Redis() = default;

    ~Redis();

    bool connect(std::string ip = "127.0.0.1", int port = 6379);

    bool publish(long channel, std::string message);

    bool subscribe(long channel);

    bool unsubscribe(long channel);

    void observerChannelMessage();

    void initNotifyHandler(std::function<void(long, std::string)> func);

private:
    redisContext* publishContext_ = nullptr;
    redisContext* subscribeContext_ = nullptr;
    std::function<void(long, std::string)> notifyMsgHandler_;
};

#endif // REDIS_HPP