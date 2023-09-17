#include "connectionpool.hpp"
#include <fstream>
#include <json.hpp>
#include <muduo/base/Logging.h>

using namespace std;
using namespace nlohmann;

ConnectionPool::ConnectionPool()
{
    if (!parseJson())
    {
        return;
    }
    for (int i = 0; i < minSize_; ++i)
    {
        addConnection();
        ++queCount_;
    }
    thread produce(&ConnectionPool::produceConnection, this);
    thread recycle(&ConnectionPool::recycleConnection, this);
    isRunning = true;
    produce.detach();
    recycle.detach();
}

void ConnectionPool::addConnection()
{
    Connection* conn = new Connection;
    conn->connect(user_, password_, database_, ip_);
    conn->resetAliveTime();
    connectionQue_.push(conn);
}

ConnectionPool* ConnectionPool::instance()
{
    static ConnectionPool pool;
    return &pool;
}

bool ConnectionPool::parseJson()
{
    ifstream ifs("dbconfig.json");
    if (!ifs.is_open())
    {
        LOG_INFO << "dbconfig.json is not exist";
        return false;
    }
    json js;
    ifs >> js;
    ip_       = js["ip"];
    user_     = js["user"];
    password_ = js["password"];
    database_ = js["database"];
    minSize_  = js["minSize"];
    maxSize_  = js["maxSize"];
    timeout_  = js["timeout"];
    maxIdelTime_ = js["maxIdelTime"];
    return true;
}

shared_ptr<Connection> ConnectionPool::getConnection()
{
    unique_lock<mutex> lock(queMtx_);
    while (connectionQue_.empty() && isRunning)
    {
        cond_.wait(lock);
    }
    shared_ptr<Connection> connection(connectionQue_.front(), [this](Connection* ptr)
    {
        lock_guard<mutex> lock(queMtx_);
        ptr->resetAliveTime();
        connectionQue_.push(ptr);
    });
    connectionQue_.pop();
    cond_.notify_all();
    return connection;
}

void ConnectionPool::produceConnection()
{
    while (isRunning)
    {
        unique_lock<mutex> lock(queMtx_);
        while (queCount_ > minSize_ && isRunning)
        {
            cond_.wait(lock);
        }
        addConnection();
        ++queCount_;
        cond_.notify_all();
    }
}

void ConnectionPool::recycleConnection()
{
    while (isRunning)
    {
        this_thread::sleep_for(chrono::milliseconds(500));
        lock_guard<mutex> lock(queMtx_);
        while (queCount_ > minSize_ && isRunning)
        {
            Connection* conn = connectionQue_.front();
            if (conn->getAliveTime() > maxIdelTime_)
            {
                if (conn) 
                {
                    delete conn;
                }
                connectionQue_.pop();
                --queCount_;
                cond_.notify_all();
            }
            else
            {
                break;
            }
        }
    }
}

ConnectionPool::~ConnectionPool()
{
    lock_guard<mutex> lock(queMtx_);
    isRunning = false;
    cond_.notify_all();
    while (connectionQue_.size())
    {
        Connection* conn = connectionQue_.front();
        connectionQue_.pop();
        if (conn)
        {
            delete conn;
        }
    }
}