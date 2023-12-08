#ifndef USER_HPP
#define USER_HPP

#include <iostream>
#include <unistd.h>

class User
{
    private:
        int _fd;

    public:
        User(int fd);
        ~User();

        int     getFd() const;
        void    closeSocket();

        void    parse(std::string msg);
        bool operator==(const User& other) const { // if find_if is not in cpp 98, change it
            return (this->_fd == other._fd);
        }
};



#endif