#include "User.hpp"

User::User(int fd) : _fd(fd)
{
    this->_isAuth = false;
    this->_nickname = "";
    this->_username = "";
    this->_realname = "";
}

User::~User()
{
    this->incomingmsg.clear();
    this->outgoingmsg.clear();
    closeSocket();
}

void User::addMessage(std::string msg)
{
    this->incomingmsg.push_back(msg);
}

std::deque<std::string> User::getMessageDeque()
{
    return (this->messageDeque);
}

int User::getFd() const
{
    return (_fd);
}

std::string User::getNickname() const
{
    return (_nickname);
}

std::string User::getUsername() const
{
    return (_username);
}

bool User::getIsAuth() const
{
    return (_isAuth);
}

void User::setNickname(std::string nickname)
{
    if (nickname.length() > 9)
        nickname = nickname.substr(0, 8) + ".";
    this->_nickname = nickname;
}

void User::setUsername(std::string username)
{
    if (username.length() > 9)
        username = username.substr(0, 8) + ".";
    this->_username = username;
}

void User::setIsAuth(bool isAuth)
{
    this->_isAuth = isAuth;
}

void User::closeSocket()
{
    if (_fd != -1)
    {
        close(_fd);
        _fd = -1;
    }
}

void User::parse(std::string msg)
{
    (void)msg;
}