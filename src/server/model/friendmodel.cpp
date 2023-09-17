#include "friendmodel.hpp"
#include "connectionpool.hpp"
#include <boost/format.hpp>

bool FriendModel::insert(long userId, long friendId)
{
    boost::format fmt("INSERT INTO friend VALUES(%1%, %2%)");
    std::string sql = boost::str(fmt % userId % friendId);
    auto mysql = ConnectionPool::instance()->getConnection();
    if (mysql->update(sql))
    {
        return true;
    }
    return false;
}

std::vector<User> FriendModel::query(long userId)
{
    boost::format fmt("SELECT id, name, state " 
                      "FROM user "
                      "INNER JOIN friend "
                      "ON user.id = friend.friendid "
                      "WHERE userid = %1%");
    std::string sql = boost::str(fmt % userId);
    auto mysql = ConnectionPool::instance()->getConnection();
    std::vector<User> vec{};
    if (mysql->query(sql))
    {
        while (mysql->next())
        {
            User user;
            user.setId(std::stol(mysql->value(0)));
            user.setName(mysql->value(1));
            user.setState(mysql->value(2));
            vec.emplace_back(user);
        }
    }
    return vec;
}