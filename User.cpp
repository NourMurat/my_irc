#include "User.hpp"

User::User(int fd) : _fd(fd) {}

User::~User() {
    closeSocket();
}

int     User::getFd() const {
    return _fd;
}

void    User::closeSocket() {
    if (_fd != -1) {
        close(_fd);
        _fd = -1;
    }
}

void    User::parse(std::string msg) {
    (void)msg;
}