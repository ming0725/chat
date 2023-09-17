#include "mysqlconn.hpp"
#include <muduo/base/Logging.h>

using namespace std;
using namespace chrono;

MysqlConn::MysqlConn()
{
    m_conn = mysql_init(nullptr);
    mysql_set_character_set(m_conn, "utf8");
}

MysqlConn::~MysqlConn()
{
    if (m_conn != nullptr)
    {
        mysql_close(m_conn);
    }
    freeResult();
}

bool MysqlConn::connect(std::string user, std::string password,
                        std::string database, std::string ip,
                        unsigned short port)
{
    MYSQL *ptr = mysql_real_connect(m_conn,
                                    ip.c_str(),
                                    user.c_str(), 
                                    password.c_str(),
                                    database.c_str(), 
                                    port,
                                    nullptr,
                                    0);
    return ptr != nullptr;
}

bool MysqlConn::update(std::string sql)
{
    if (mysql_query(m_conn, sql.c_str()))
    {
        LOG_INFO << "SQL ERROR: " << sql;
        return false;
    }
    return true;
}

bool MysqlConn::query(std::string sql)
{
    freeResult();
    if (mysql_query(m_conn, sql.c_str()))
    {
        LOG_INFO << "SQL ERROR: " << sql;
        return false;
    }
    m_result = mysql_store_result(m_conn);
    return true;
}

bool MysqlConn::next()
{
    m_row = mysql_fetch_row(m_result);
    if (m_row != nullptr)
    {
        return true;
    }
    return false;
}

std::string MysqlConn::value(int index)
{
    int fields = mysql_num_fields(m_result);
    if (index >= fields || index < 0)
    {
        return std::string();
    }
    char *val = m_row[index];
    unsigned long length = mysql_fetch_lengths(m_result)[index];
    return std::string(val, length);
}

void MysqlConn::freeResult()
{
    if (m_result)
    {
        mysql_free_result(m_result);
        m_result = nullptr;
    }
}

int MysqlConn::getField()
{
    if (m_result == nullptr)
    {
        return 0;
    }
    return mysql_num_fields(m_result);
}

std::string MysqlConn::getError() {
    const char* error = mysql_error(m_conn);
    if (error)
    {
        return std::string(error);
    }
    return std::string();
}

unsigned long long MysqlConn::getInserId()
{
    return mysql_insert_id(m_conn);
}

void MysqlConn::resetAliveTime()
{
    aliveTime_ = steady_clock::now();
}

long long MysqlConn::getAliveTime()
{
    // nanoseconds res = steady_clock::now() - aliveTime_;
    // milliseconds millisec = duration_cast<milliseconds>(res);
    // return millisec.count();

    return duration_cast<milliseconds>(nanoseconds(steady_clock::now() - aliveTime_)).count();
}