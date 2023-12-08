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

const int PORT = 6667;
const int MAX_CLIENTS = 4096;

class Server
{
    private:
        int _fd;
        char _buffer[4096];
        size_t _bufferLength;

    public:
        Server();
        Server(int fd);
        ~Server();

        int fdVal() const;
        int getFd() const;
        void setFd(int fd);
        
        char* getBuffer();
        void clearBuffer();
        
        size_t getBufferLen() const;
        void setBufferLen(size_t len);

        int createSocket();
        void bindSocket(int sockfd);
        void listenSocket(int sockfd);
        int acceptConection(int sockfd);
        void removeUser(std::vector<Server>& users, int fd);
        void runServer();
};

struct FindByFD
{
    int fd;

    FindByFD(int fd) : fd(fd) { }
    bool operator()(const Server &user) const {
        return user.getFd() == fd;
    }
};

#endif