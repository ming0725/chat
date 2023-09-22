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
    isRunning_ = true;
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
    while (connectionQue_.empty())
    {
        cond_.notify_all();
        if (cv_status::timeout == cond_.wait_for(lock, chrono::milliseconds(timeout_)))
        {
            if (connectionQue_.empty())
            {
                LOG_INFO << "acquire connection timeout!";
                return nullptr;
            }
            else 
            {
                break;
            }
        }
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
    while (isRunning_)
    {
        unique_lock<mutex> lock(queMtx_);
        while (queCount_ > minSize_)
        {
            cond_.wait(lock);
            if (!isRunning_) 
            {
                return;
            }
        }
        addConnection();
        ++queCount_;
        cond_.notify_all();
    }
}

void ConnectionPool::recycleConnection()
{
    while (isRunning_)
    {
        this_thread::sleep_for(chrono::milliseconds(maxIdelTime_));
        lock_guard<mutex> lock(queMtx_);
        while (queCount_ > maxSize_)
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
    isRunning_ = false;
    cond_.notify_all();
    lock_guard<mutex> lock(queMtx_);
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