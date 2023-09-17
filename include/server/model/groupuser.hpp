#ifndef GROUPUSER_HPP
#define GROUPUSER_HPP

#include "user.hpp"

class GroupUser : public User
{
private:
    std::string role;
public:
    std::string& getRole()          { return this->role; }
    void setRole(std::string role)  { this->role = role; }
};

#endif // GROUPUSER_HPP