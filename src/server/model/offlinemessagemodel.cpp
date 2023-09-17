#include "offlinemessagemodel.hpp"
#include "connectionpool.hpp"
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;

bool OfflineMessageModel::insert(long userId, std::string msg)
{
    boost::format fmt("INSERT INTO offlinemessage(userid, message) VALUES(%1%, '%2%')");
    boost::replace_all(msg, "'", "\\'");
    boost::replace_all(msg, "\\", "\\\\");
    string sql = boost::str(fmt % userId % msg);
    auto mysql = ConnectionPool::instance()->getConnection();
    if (mysql->update(sql))
    {
        return true;
    }
    return false;
}

bool OfflineMessageModel::remove(long userId)
{
    boost::format fmt("DELETE FROM offlinemessage WHERE userid = %1%");
    string sql = boost::str(fmt % userId);
    auto mysql = ConnectionPool::instance()->getConnection();
    if (mysql->update(sql))
    {
        return true;
    }
    return false;
}

vector<string> OfflineMessageModel::query(long userId)
{
    boost::format fmt("SELECT * FROM offlinemessage WHERE userid = %1%");
    string sql = boost::str(fmt % userId);
    vector<string> ret{};
    auto mysql = ConnectionPool::instance()->getConnection();
    if (mysql->query(sql))
    {
        while (mysql->next())
        {
            ret.emplace_back(mysql->value(1));
        }
    }
    return ret;
}
