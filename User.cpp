#include "User.hpp"
#include "Server.hpp"

User::User(int fd) : _fd(fd) {
    this->_isAuth = false;
    this->_isOP = false;
    this->_buffer = "";
    this->_nickname = "";
    this->_username = "";
    this->_userHost = "";
    this->_realname = "";
}

User::~User() {
    this->_incomingMsgs.clear();
    // this->outgoingmsg.clear();
    closeSocket();
}

//=================================<USER RECEIVES MSG>=============================================================

size_t      User::receiveMsg() {
    char buffer[4096]; // Create a buffer to store incoming data
    size_t byteRead = read(_fd, buffer, sizeof(buffer)); // Read data from the socket

    if (byteRead <= 0) {
        return byteRead; // If no data or an error, return the number of bytes read
    }

    buffer[byteRead] = '\0'; // Add a null terminator at the end of the string

    _buffer.clear();
    _buffer = buffer; // Save the read data in the class variable

    // Process the read data
    splitAndProcess(_buffer);
    return byteRead; // Return the number of bytes read
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

    // Process the last fragment after the last "\r\n" or if the string haven`t "\r\n"
    if (start < data.length()) {
        std::istringstream iss(data.substr(start));
        std::string word;

        while (iss >> word) {
            _incomingMsgs.push_back(word);
        }
    }
}

//---------------------------------------------------------------------------------------------

void            User::addMessage(std::string msg)
{
    this->_incomingMsgs.push_back(msg);
}

// std::deque<std::string> User::getMessageDeque()
// {
//     return (this->messageDeque);
// }

int             User::getFd() const { return (_fd); }
std::string     User::getNickname() const { return (_nickname); }
std::string     User::getUsername() const { return (_username); }
std::string     User::getUserHost() const { return (_userHost); }
std::string     User::getBuffer() const { return _buffer; }
bool            User::getIsAuth() const { return (_isAuth); }

void            User::setNickname(std::string nickname) { this->_nickname = nickname; }
void            User::setUsername(std::string username) { this->_username = username; }
void            User::setUserHost(std::string userHost) { this->_userHost = userHost; }
void            User::setIsAuth(bool isAuth) { this->_isAuth = isAuth; }

void            User::closeSocket()
{
    if (_fd != -1)
    {
        close(_fd);
        _fd = -1;
    }
}

void            User::parse(std::string msg)
{
    (void)msg;
}

