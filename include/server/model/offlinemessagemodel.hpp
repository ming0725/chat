#ifndef OFFLINEMESSAGEMODEL_HPP
#define OFFLINEMESSAGEMODEL_HPP

#include <vector>
#include <string>

class OfflineMessageModel
{
public:
    bool insert(long userId, std::string msg);

    bool remove(long userId);

    std::vector<std::string> query(long userId);
};

#endif // OFFLINEMESSAGEMODEL_HPP