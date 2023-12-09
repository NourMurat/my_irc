#ifndef SERVER_HPP
#define SERVER_HPP

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m" 
#define MAGENTA "\033[35m"   
#define CYAN    "\033[36m" 
#define RESET	"\033[0m"

// #include <poll.h>
// #include <iostream>
// #include <sys/socket.h>
// #include <vector>

#include <vector>
#include <poll.h>
#include <fcntl.h>
#include <netdb.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <sys/types.h>
#include <arpa/inet.h> //htons
#include <arpa/inet.h>
#include <sys/socket.h>
#include "User.hpp"
#include "signal.h"

// const int PORT = 6666;
const int MAX_CLIENTS = 4096;
class User;

class Server;
extern Server* globalServerInstance; // to use static signal and shutdown functions

class Server
{
    private:
        char            _buffer[4096];

        std::string     _password;
        int             _port;
        bool            _running;

    public:
        Server();
        Server (const int& port, const std::string& password);
        ~Server();

        void runServer();
        int createSocket();
        void bindSocket(int sockfd);
        void listenSocket(int sockfd);
        int acceptConection(int sockfd);
        void removeUser(std::vector<User>& users, int fd);

        std::vector<User> _users;
        std::vector<pollfd> _fds;

        static void sigIntHandler(int signal);
        static void sigTermHandler(int signal);
        static void shutdownServer();
};

struct FindByFD
{
    int fd;

    FindByFD(int fd) : fd(fd) { }
    bool operator()(const User &user) const {
        return user.getFd() == fd;
    }
    bool operator()(const struct pollfd& pfd) const {
        return pfd.fd == fd;
    }
};

#endif