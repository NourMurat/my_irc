#ifndef USER_HPP
#define USER_HPP

#include <iostream>

class User
{
    public:
        int _fd;
        User(int fd);
        ~User();
        void parse(std::string msg);
        bool operator==(const User& other) const { // if find_if is not in cpp 98, change it
            return (this->_fd == other._fd);
        }
};



#endif