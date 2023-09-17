#ifndef USERMODEL_HPP
#define USERMODEL_HPP

#include "user.hpp"

class UserModel
{
public:
    // 添加用户
    bool insert(User& user);

    // 根据id查询用户信息
    User query(long id);

    // 更新用户状态
    bool updateState(User& user);

    // 重置用户状态
    bool resetState(User& user);

    // 更新用户信息
    bool update(User& user);
};

#endif // USERMODEL_HPP