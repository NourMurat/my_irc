#include "User.hpp"

User::User(int fd) { 
    this->_fd = fd;
}

User::~User() {

}

void User::parse(std::string msg) {
    (void)msg;
}