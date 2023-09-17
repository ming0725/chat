#ifndef USER_HPP
#define USER_HPP

#include <string>

class User
{
public:
    User(long id, std::string name, std::string password = "123456", std::string state = "offline")
        : id(id), name(name), password(password), state(state) {}

    User() = default;

    void setId      (long id)               { this->id = id; }
    void setName    (std::string name)      { this->name = name; }
    void setPassword(std::string password)  { this->password = password; }
    void setState   (std::string state)     { this->state = state; }

    long getId()                 { return this->id; }
    std::string& getName()       { return this->name; }
    std::string& getPassword()   { return this->password; }
    std::string& getState()      { return this->state; }

private:
    long id;
    std::string name;
    std::string password;
    std::string state;
};

#endif // USER_HPP