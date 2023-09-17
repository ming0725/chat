#include "groupmodel.hpp"
#include "connectionpool.hpp"
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>

// 创建群组
bool GroupModel::createGroup(Group& group)
{
    boost::format fmt("INSERT INTO allgroup(groupname, groupdesc) VALUES('%1%', '%2%')");
    boost::replace_all(group.getName(), "\\", "\\\\");
    boost::replace_all(group.getName(), "'", "\\'");
    boost::replace_all(group.getDesc(), "\\", "\\\\");
    boost::replace_all(group.getDesc(), "'", "\\'");
    std::string sql = boost::str(fmt % group.getName() % group.getDesc());
    auto mysql = ConnectionPool::instance()->getConnection();
    if (mysql->update(sql))
    {
        group.setId(mysql->getInserId());
        return true;
    }
    return false;
}

bool GroupModel::addGroup(long userId, long groupId, std::string role)
{
    boost::format fmt("INSERT INTO groupuser(groupid, userid, grouprole) VALUES(%1%, %2%, '%3%')");
    boost::replace_all(role, "'", "\\'");
    boost::replace_all(role, "\\", "\\\\");
    std::string sql = boost::str(fmt % groupId % userId % role);
    auto mysql = ConnectionPool::instance()->getConnection();
    if (mysql->update(sql))
    {
        return true;
    }
    return false;
}

// 查询用户所在群组信息
std::vector<Group> GroupModel::queryGroups(long userId)
{
    boost::format fmt("SELECT allgroup.* "
                      "FROM allgroup "
                      "INNER JOIN groupuser "
                      "ON groupuser.groupid = allgroup.id "
                      "WHERE groupuser.userid = %1%");
    std::string sql = boost::str(fmt % userId);
    auto mysql = ConnectionPool::instance()->getConnection();
    std::vector<Group> vec{};
    if (mysql->query(sql))
    {
        while (mysql->next())
        {
            Group group(std::stol(mysql->value(0)), mysql->value(1), mysql->value(2));
            vec.emplace_back(group);
        }
    }
    return vec;
}

// 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
std::vector<long> GroupModel::queryGroupUsers(long userId, long groupId)
{
    boost::format fmt("SELECT userid FROM groupuser WHERE userid != %1% AND groupid = %2%");
    std::string sql = boost::str(fmt % userId % groupId);
    auto mysql = ConnectionPool::instance()->getConnection();
    std::vector<long> vec{};
    if (mysql->query(sql))
    {   
        while (mysql->next())
        {
            vec.emplace_back(std::stol(mysql->value(0))); 
        }
    }
    return vec;
}

// 查询某个群组信息
Group GroupModel::queryGroupInfo(long groupId)
{
    boost::format fmt("SELECT * "
                      "FROM allgroup "
                      "WHERE id = %1%");
    std::string sql = boost::str(fmt % groupId);
    auto mysql = ConnectionPool::instance()->getConnection();
    if (mysql->query(sql) && mysql->next())
    {
        return Group(std::stol(mysql->value(0)), mysql->value(1), mysql->value(2));
    }
    return Group();
}