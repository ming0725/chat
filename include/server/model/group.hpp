#ifndef GROUP_HPP
#define GROUP_HPP

#include <string>
#include "groupuser.hpp"

class Group
{
public:
    Group(long id, std::string name, std::string desc)
        : id(id), name(name), desc(desc) {}

    Group() = default;

    long getId()           { return this->id; }
    std::string& getName() { return this->name; }
    std::string& getDesc() { return this->desc; }

    void setId(long id)            { this->id = id; }
    void setName(std::string name) { this->name = name; }
    void setDesc(std::string desc) { this->desc = desc; } 

private:
    long id;
    std::string name;
    std::string desc;
    std::vector<GroupUser> users;
};
#endif // GROUP_HPP