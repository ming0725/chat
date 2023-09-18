#ifndef CONNECTIONPOOL_HPP
#define CONNECTIONPOOL_HPP

#include <queue>
#include <atomic>
#include <mutex>
#include <thread>
#include <memory>
#include <condition_variable>
#include "mysqlconn.hpp"

using Connection = MysqlConn;

class ConnectionPool
{
public:
    static ConnectionPool* instance();

    bool parseJson();

    std::shared_ptr<Connection> getConnection();

    ~ConnectionPool();

public:
    ConnectionPool(const ConnectionPool&) = delete;
    ConnectionPool& operator=(const ConnectionPool&) = delete;

private:
    ConnectionPool();

    void addConnection();

    void produceConnection();

    void recycleConnection();

private:
    std::string ip_;
    std::string user_;
    std::string password_;
    std::string database_;
    int minSize_;
    int maxSize_;
    int maxIdelTime_;
    int timeout_;

private:
    std::queue<Connection*> connectionQue_;
    std::mutex queMtx_;
    std::condition_variable cond_;
    std::atomic_int queCount_;
    std::atomic_bool isRunning_;
};

#endif // CONNECTIONPOOL_HPP