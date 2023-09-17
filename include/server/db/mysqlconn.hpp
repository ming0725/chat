#ifndef MYSQLCONNECTOR_HPP
#define MYSQLCONNECTOR_HPP

#include <mysql/mysql.h>
#include <string>
#include <chrono>

class MysqlConn
{
public:
    MysqlConn();

    ~MysqlConn();

    bool connect(std::string user, std::string password, std::string database,
                 std::string ip, unsigned short port = 3306);

    bool update(std::string sql);

    bool query(std::string sql);

    bool next();

    std::string value(int index);

    int getField();

    std::string getError();

    unsigned long long getInserId();

    void resetAliveTime();

    long long getAliveTime();

private:
    void freeResult();

private:
    std::chrono::steady_clock::time_point aliveTime_;

private:
    MYSQL *m_conn = nullptr;
    MYSQL_RES *m_result = nullptr;
    MYSQL_ROW m_row = nullptr;
};
#endif //MYSQLCONNECTOR_HPP
