#include "usermodel.hpp"
#include "connectionpool.hpp"
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

bool UserModel::insert(User &user)
{
    boost::format fmt("INSERT INTO user(name, password, state) VALUES('%1%', '%2%', '%3%')");
    boost::replace_all(user.getPassword(), "\\", "\\\\");
    boost::replace_all(user.getPassword(), "'", "\\'");
    boost::replace_all(user.getName(), "\\", "\\\\");
    boost::replace_all(user.getName(), "'", "\\'");
    std::string sql = boost::str(fmt % user.getName() % user.getPassword() % user.getState());
    auto mysql = ConnectionPool::instance()->getConnection();
    if (mysql->update(sql))
    {
        user.setId(mysql->getInserId());
        return true;
    }
    return false;
}

// 根据id查询用户信息
User UserModel::query(long id)
{
    boost::format fmt("SELECT * FROM user WHERE id = %1%");
    std::string sql = boost::str(fmt % id);
    auto mysql = ConnectionPool::instance()->getConnection();
    if (mysql->query(sql) && mysql->next())
    {
        return User(std::stol(mysql->value(0)), mysql->value(1), mysql->value(2), mysql->value(3));
    }
    return User();
}

// 更新用户状态
bool UserModel::updateState(User &user)
{
    boost::format fmt("UPDATE user SET state = 'online' WHERE id = %1%");
    std::string sql = boost::str(fmt % user.getId());
    auto mysql = ConnectionPool::instance()->getConnection();
    if (mysql->update(sql))
    {
        return true;
    }
    return false;
}

// 重置用户状态
bool UserModel::resetState(User &user)
{
    boost::format fmt("UPDATE user SET state = 'offline' WHERE id = %1%");
    std::string sql = boost::str(fmt % user.getId());
    auto mysql = ConnectionPool::instance()->getConnection();
    if (mysql->update(sql))
    {
        return true;
    }
    return false;
}

// 更新用户信息
bool UserModel::update(User &user)
{
    boost::format fmt("UPDATE user SET name = '%1%', password = '%2%', state = '%3%' WHRER id = '%4%'");
    std::string sql = boost::str(fmt % user.getName() % user.getPassword() % user.getState() % user.getId());
    boost::replace_all(user.getName(), "\\", "\\\\");
    boost::replace_all(user.getName(), "'", "\\'");
    boost::replace_all(user.getPassword(), "\\", "\\\\");
    boost::replace_all(user.getPassword(), "'", "\\'");
    boost::replace_all(user.getState(), "\\", "\\\\");
    boost::replace_all(user.getState(), "'", "\\'");
    auto mysql = ConnectionPool::instance()->getConnection();
    if (mysql->update(sql))
    {
        return true;
    }
    return false;
}
