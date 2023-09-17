#ifndef GROUPMODEL_HPP
#define GROUPMODEL_HPP

#include <vector>
#include "group.hpp"

class GroupModel
{
public:
    // 创建群组
    bool createGroup(Group& group);

    // 加入群组
    bool addGroup(long userId, long groupId, std::string role);

    // 查询用户所在群组信息
    std::vector<Group> queryGroups(long userId);

    // 根据指定的groupid查询群组用户id列表，除userid自己，主要用户群聊业务给群组其它成员群发消息
    std::vector<long> queryGroupUsers(long userId, long groupId);

    // 查询某个群组信息
    Group queryGroupInfo(long groupId);
};

#endif // GROUPMODEL_HPP