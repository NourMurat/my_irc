#include "Server.hpp"

Server *globalServerInstance = NULL; // to use static signal and shutdown functions

Server::Server() : _buffer("\0"), _port(0), _running(false)
{
    std::cout << GREEN << "Server Default Constructor Called!" << RESET << std::endl;
    globalServerInstance = this;
}

Server::Server(const int &port, const std::string &password) : _buffer("\0"), _password(password), _port(port), _running(false)
{
    std::cout << GREEN << "Server Parameter Constructor has called!" << RESET << std::endl;
    globalServerInstance = this;
}

Server::~Server()
{
    std::cout << RED << "Server Destructor Called" << RESET << std::endl;
    // shutdownServer();
}

//===============================<START>========================================================

void Server::runServer()
{
    int sockfd = createSocket();
    int optval = 1;

    signal(SIGINT, Server::sigIntHandler);
    signal(SIGTERM, Server::sigTermHandler);
    _running = true;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&optval), sizeof(optval)) < 0)
    {
        throw std::runtime_error("ERROR! Socket options error!\n");
    }
    if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
    {
        throw std::runtime_error("ERROR! File control error!\n");
    }

    bindSocket(sockfd);
    listenSocket(sockfd);

    pollfd tmp = {sockfd, POLLIN, 0};
    _fds.push_back(tmp);

    while (_running)
    {
        if (!_running)
            break;
        // int poll(representing a FD, number of FD, timeout);
        int numFds = poll(_fds.data(), _fds.size(), -1);
        if (numFds == -1)
        {
            throw std::runtime_error("ERROR! Poll error!\n");
        }
        for (int i = 0; i < (int)_fds.size(); i++)
        {
            if (_fds[i].revents & POLLIN)
            { // data that can be read without blocking AND can safely read operation be on it
                if (_fds[i].fd == sockfd)
                {
                    // New client connection and add it to "users, _fds" vectors
                    int clientFd = acceptConection(sockfd);
                    pollfd tmp2 = {clientFd, POLLIN, 0};
                    _fds.push_back(tmp2);
                    _users.push_back(User(clientFd));
                    send(clientFd, "CAP * ACK multi-prefix\r\n", 27, 0);
                    std::cout << BLUE << "new client connected FD:" << clientFd << RESET << std::endl;
                }
                else
                {
                    // Client message received
                    int byteRead = read(_fds[i].fd, _buffer, sizeof(_buffer));
                    _buffer[byteRead - 1] = '\0';

                    std::cout << "---------> " << byteRead << std::endl;
                    if (byteRead <= 0)
                    {
                        std::cout << RED << "Client disconnected FD:" << _fds[i].fd << RESET << std::endl
                                  << std::flush;
                        removeUser(_users, _fds[i].fd);
                        _fds.erase(_fds.begin() + i);
                        i--;
                    }
                    else
                    {
                        std::vector<User>::iterator it = std::find_if(_users.begin(), _users.end(), FindByFD(_fds[i].fd));

                        std::string strBuffer(_buffer);
                        std::cout << BLUE << "Received message from client" << _fds[i].fd << ": " << RESET << strBuffer << std::endl;
                        it->parse(strBuffer);
                    }
                }
            }
        }
    }
}

//===================================<METHODS>====================================================

int Server::createSocket()
{
    // int socket(int domain, int type, int protocol);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        std::cerr << RED "Failed to create socket" << RESET << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << GREEN << "Server Socket Created FD:" << sockfd << RESET << std::endl;
    return sockfd;
}

void Server::bindSocket(int sockfd)
{
    struct sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;                   //(0.0. 0.0) any address for binding
    serverAddr.sin_port = htons(static_cast<uint16_t>(_port)); // convert to network byte order.

    // int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        std::cerr << RED << "Failed to bind socket." << RESET << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << GREEN << "Bind socket has been successfully bound." << RESET << std::endl;
}

void Server::listenSocket(int sockfd)
{
    if (listen(sockfd, MAX_CLIENTS) == -1)
    {
        std::cerr << RED << "Failed to listen socket." << RESET << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << GREEN << "Successfully listen " << RESET << std::endl;
}

int Server::acceptConection(int sockfd)
{
    struct sockaddr_in clientAddr; // hold clientAddr information
    socklen_t clientLen = sizeof(clientAddr);
    // int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
    int clientFd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientLen);
    if (clientFd == -1)
    {
        std::cerr << RED << "Failed to accept << " << RESET << std::endl;
        exit(EXIT_FAILURE);
    }
    std::cout << GREEN << "Successfully accept " << RESET << std::endl;
    return clientFd; // Return the new socket descriptor for communication with the client.
}

// void Server::createNewUser(int fd);
// {
//     _users.push_back(new User(fd));
//     std::cout << BLUE << "new client connected FD:" << fd << RESET << std::endl;
// }

void Server::removeUser(std::vector<User> &users, int fd)
{
    // Удаление пользователя из списка пользователей
    std::vector<User>::iterator itUser = std::find_if(users.begin(), users.end(), FindByFD(fd));
    if (itUser != users.end())
    {
        itUser->closeSocket();
        users.erase(itUser);
    }
    // Удаление файлового дескриптора из _fds
    std::vector<struct pollfd>::iterator itFd = std::find_if(_fds.begin(), _fds.end(), FindByFD(fd));
    if (itFd != _fds.end())
    {
        _fds.erase(itFd);
    }
}

//====================================<SIGNALS && SHUTDOWN>====================================

// Обработчик для SIGINT
void Server::sigIntHandler(int signal)
{
    std::cout << MAGENTA << "\nReceived SIGINT (Ctrl+C) signal: " << signal << RESET << std::endl;
    if (globalServerInstance)
    {
        globalServerInstance->shutdownServer();
    } // Вызов метода для корректного завершения работы сервера
}

// Обработчик для SIGTERM
void Server::sigTermHandler(int signal)
{
    std::cout << MAGENTA << "\nReceived SIGTERM signal: " << signal << RESET << std::endl;
    if (globalServerInstance)
    {
        globalServerInstance->shutdownServer();
    } // Вызов метода для корректного завершения работы сервера
}

// Метод для корректного завершения работы сервера
void Server::shutdownServer()
{
    std::cout << CYAN << "Shutting down server..." << RESET << std::endl;
    if (globalServerInstance)
    {
        globalServerInstance->_running = false;
        for (std::vector<struct pollfd>::iterator it = globalServerInstance->_fds.begin(); it != globalServerInstance->_fds.end(); ++it)
        {
            // Закрытие каждого сокета в _fds
            if (it->fd != -1)
            {
                close(it->fd);
                it->fd = -1; // Устанавливаем файловый дескриптор в -1 после закрытия
            }
        }
        globalServerInstance->_fds.clear(); // Очистка списка файловых дескрипторов после закрытия всех сокетов
        std::cout << CYAN << "Server successfully shut down!" << RESET << std::endl;

    }
}
