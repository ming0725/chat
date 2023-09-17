#ifndef FRIENDMODEL_HPP
#define FRIENDMODEL_HPP

#include <vector>
#include <string>
#include "user.hpp"

class FriendModel
{
public:
    bool insert(long userId, long friendId);

    // 返回好友列表
    std::vector<User> query(long userId);
};

#endif // FRIENDMODEL_HPP