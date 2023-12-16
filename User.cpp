#include "User.hpp"
#include "Server.hpp"

User::User(int fd) : _fd(fd) {
    this->_isAuth = false;
    this->_isOP = false;
    this->_buffer = "";
    this->_nickname = "";
    this->_username = "";
    this->_realname = "";
}

User::~User() {
    this->_incomingMsgs.clear();
    // this->outgoingmsg.clear();
    closeSocket();
}

//==============================================================================================

size_t      User::receiveMsg() {
    char buffer[4096]; // Create a buffer to store incoming data
    size_t bytesRead = recv(_fd, buffer, sizeof(buffer) - 1, 0); // Read data from the socket

    if (bytesRead <= 0) {
        return bytesRead; // If no data or an error, return the number of bytes read
    }

    buffer[bytesRead] = '\0'; // Add a null terminator at the end of the string

    _buffer.clear();
    _buffer = buffer; // Save the read data in the class variable

    // Process the read data
    splitAndProcess(_buffer);
    return bytesRead; // Return the number of bytes read
}

void        User::splitAndProcess(const std::string& data) {
    std::string::size_type start = 0; // Start position for search
    std::string::size_type end; // End position for search

    _incomingMsgs.clear();
    // Loop to find and process substrings separated by "\r\n"
    while ((end = data.find("\r\n", start)) != std::string::npos) {
        std::string fragment = data.substr(start, end - start); // Extract the substring

        std::istringstream iss(fragment); // Use stringstream to split by spaces
        std::string word;

        // Split the substring into words
        while (iss >> word) {
            _incomingMsgs.push_back(word); // Add words to the _incomingMsgs
        }

        start = end + 2; // Skip the "\r\n" characters for the next search
    }

    // Process the last fragment after the last "\r\n"
    if (start < data.length()) {
        std::istringstream iss(data.substr(start));
        std::string word;

        while (iss >> word) {
            _incomingMsgs.push_back(word); // Add remaining words
        }
    }

}

//---------------------------------------------------------------------------------------------

void User::addMessage(std::string msg)
{
    this->_incomingMsgs.push_back(msg);
}

// std::deque<std::string> User::getMessageDeque()
// {
//     return (this->messageDeque);
// }

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

bool User::getIsOP() const
{
    return (_isOP);
}

void User::setIsOP(bool isOP)
{
    this->_isOP = isOP;
}

void User::setNickname(std::string nickname)
{
    if (nickname.length() > 9)
        nickname = nickname.substr(0, 9) + ".";
    this->_nickname = nickname;
}

void User::setUsername(std::string username)
{
    if (username.length() > 9)
        username = username.substr(0, 9) + ".";
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

std::string     User::getBuffer() {
    return _buffer;
}
