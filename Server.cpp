#include "Server.hpp"

Server *globalServerInstance = NULL; // to use static signal and shutdown functions

Server::Server() : _port(0), _disconnect(true)
{
	std::cout << GREEN << "Server Default Constructor Called!" << RESET << std::endl;
	globalServerInstance = this;
}

std::string Server::getServerName() const
{
	return this->_serverName;
}

void Server::setServerName(std::string serverName)
{
	this->_serverName = serverName;
}

Server::Server(const int &port, const std::string &password) : _serverName(""), _password(password), _port(port), _disconnect(true)
{
	std::cout << GREEN << "Server Parameter Constructor has called!" << RESET << std::endl;
	globalServerInstance = this;
}

Server::~Server()
{
	std::cout << GREEN << "Server Destructor Called" << RESET << std::endl;
	// shutdownServer();
}

//===============================<START>========================================================

int checkDupNickname(std::vector<User *> users, std::string nickname)
{
	for (std::vector<User *>::iterator it = users.begin(); it != users.end(); ++it)
	{
		if ((*it)->getNickname() == nickname)
			return 1;
	}
	return 0;
}

void Server::welcomeMsg(User *user)
{

	std::string welcomeMsg;
	welcomeMsg = ":IRC 001 " + _serverName + " :Welcome to the Internet Relay Network " + user->getNickname() + "\r\n";
	send(user->getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
	welcomeMsg = ":IRC 002 " + _serverName + " :Your host is " + _serverName + ", running version V1.0\r\n";
	send(user->getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
	welcomeMsg = ":IRC 003 " + _serverName + " :This server was created in December 2023\r\n";
	send(user->getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
	welcomeMsg = ":IRC 004 " + _serverName + " :<servername> <version> <available user modes> <available channel modes>\r\n";
	send(user->getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
}

int isCommand(std::string command)
{
	if (command == "NICK" || command == "/NICK")
		return (NICK);
	if (command == "USER" || command == "/USER")
		return (USER);
	if (command == "JOIN")
		return (JOIN);
	if (command == "MSG")
		return (MSG);
	if (command == "PRIVMSG")
		return (PRIVMSG);
	if (command == "PING")
		return (PING);
	if (command == "PART")
		return (PART);
	if (command == "INVITE")
		return (INVITE);
	if (command == "TOPIC")
		return (TOPIC);
	if (command == "MODE")
		return (MODE);
	if (command == "QUIT" || command == "/QUIT")
		return (QUIT);
	return (NOTCOMMAND);
}

void Server::authenticateUser(int i)
{
	std::vector<User *>::iterator it = std::find_if(_users.begin(), _users.end(), FindByFD(_fds[i].fd));
	if (!(*it)->getIsAuth())
	{
		for (size_t i = 0; i < (*it)->_incomingMsgs.size(); ++i)
		{
			if ((*it)->_incomingMsgs[i] == "PASS")
			{
				if ((*it)->_incomingMsgs[i + 1] != _password)
				{
					std::string error = "ERROR :Wrong password\r\n";
					send((*it)->getFd(), error.c_str(), error.length(), 0);
					removeUser(_users, (*it)->getFd());
					break;
				}
			}
			if ((*it)->_incomingMsgs[i] == "NICK")
			{
				if (checkDupNickname(_users, (*it)->_incomingMsgs[i + 1]))
				{
					std::string error = "ERROR :Nickname is already in use\r\n";
					send((*it)->getFd(), error.c_str(), error.length(), 0);
					removeUser(_users, (*it)->getFd());
					break;
				}
				(*it)->setNickname((*it)->_incomingMsgs[i + 1]); // user nick
			}
			if ((*it)->_incomingMsgs[i] == "USER")
			{
				(*it)->setUsername((*it)->_incomingMsgs[i + 1]); // user name
				setServerName((*it)->_incomingMsgs[i + 3]);		 // server name
			}
			if (!(*it)->getIsAuth() && (!(*it)->getNickname().empty()) &&
				(!(*it)->getUsername().empty()) && (!getServerName().empty()))
			{
				welcomeMsg((*it));
				(*it)->setIsAuth(true);
			}
		}
	}
}

void Server::runServer()
{
	int optval = 1;
	int sockfd = createSocket();

	signal(SIGINT, Server::sigIntHandler);
	signal(SIGTERM, Server::sigTermHandler);
	bindSocket(sockfd);
	listenSocket(sockfd);

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&optval), sizeof(optval)) < 0)
		throw std::runtime_error("ERROR! Socket options error!\n");
	if (fcntl(sockfd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("ERROR! File control error!\n");
	pollfd tmp = {sockfd, POLLIN, 0};
	_fds.push_back(tmp);

	_disconnect = false;
	while (!_disconnect)
	{
		if (_disconnect)
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
					_users.push_back(new User(clientFd));
					std::cout << "User has been created with class User FD:" << _users[i]->getFd() << std::endl; // for debugging -> create class User
					std::string firstServerMsg = "CAP * ACK multi-prefix\r\n";
					send(clientFd, firstServerMsg.c_str(), firstServerMsg.length(), 0);
					std::cout << BLUE << "new client connected FD:" << clientFd << RESET << std::endl;
				}
				else
				{
					// Client message received
					int byteRead = _users[i - 1]->receiveMsg(); // read(_fds[i].fd, _buffer, sizeof(_buffer));
					std::cout << "---------> " << byteRead << std::endl;
					if (byteRead < 0)
						std::cerr << RED << "Read error on FD:" << _fds[i].fd << RESET << std::endl;
					else if (byteRead == 0)
					{
						std::cout << RED << "Client disconnected FD:" << _fds[i].fd << RESET << std::endl
								  << std::flush;
						removeUser(_users, _fds[i].fd);
						// _fds.erase(_fds.begin() + i);
						i--;
					}
					else
					{
						std::vector<User *>::iterator it = std::find_if(_users.begin(), _users.end(), FindByFD(_fds[i].fd));
						std::cout << BLUE << "Received message from client" << _fds[i].fd << ":\n"
								  << RESET << (*it)->getBuffer() << std::endl;
						// for (size_t i = 0; i < (*it)->_incomingMsgs.size(); ++i) //debugging, delete it before submit
						//     std::cout << i << ": " << (*it)->_incomingMsgs[i] << std::endl; //debugging, delete it before submit
						if (!(*it)->getIsAuth())
						{
							authenticateUser(i);
							// debugging, delete it before submit
							if (((*it)->getIsAuth() && !(*it)->getNickname().empty()) && (!(*it)->getUsername().empty()) &&
								(!getServerName().empty()))
							{
								std::cout << GREEN << "<SERVER`S DATA>"
										  << "\n";
								std::cout << "Password - " << _password << "\n";
								std::cout << "NickName - " << (*it)->getNickname() << "\n";
								std::cout << "UserName - " << (*it)->getUsername() << "\n";
								std::cout << "Server Name - " << _serverName << RESET << "\n";
							}
						}
						if (isCommand((*it)->_incomingMsgs[0]) != NOTCOMMAND)
						{
							int i = isCommand((*it)->_incomingMsgs[0]);

							switch (i)
							{
							case NICK:
							{
								(*it)->setNickname((*it)->_incomingMsgs[1]);
								std::string msg = "Your nickname has been changed to " + (*it)->getNickname() + "\r\n";
								std::cout << "nickname has been set to " << (*it)->getNickname() << "\n";
								send((*it)->getFd(), msg.c_str(), msg.length(), 0);
								break;
							}
							case USER:
							{
								(*it)->setUsername((*it)->_incomingMsgs[1]);
								std::string msg = "Your username has been changed to " + (*it)->getNickname() + "\r\n";
								std::cout << "username has been set to " << (*it)->getUsername() << std::endl;
								send((*it)->getFd(), msg.c_str(), msg.length(), 0);
								break;
							}
							case PING:
							{
								std::string pong = "PONG\r\n";
								send((*it)->getFd(), pong.c_str(), pong.length(), 0);
								std::cout << "PONG has been sent to " << (*it)->getNickname() << std::endl;
								break;
							}
							case MSG:
							case PRIVMSG:
							{
								if ((*it)->_incomingMsgs[1][0] != '#')
								{
									std::vector<User *>::iterator itReceiver = std::find_if(_users.begin(), _users.end(), FindByNickname((*it)->_incomingMsgs[1]));
									std::string msg = (*it)->_incomingMsgs[2];
									if ((*itReceiver)->getFd() != -1)
									{
										for (unsigned int index = 3; index < (*it)->_incomingMsgs.size(); ++index)
											msg += " " + (*it)->_incomingMsgs[index];
										std::string resendMsg = ":" + (*it)->getNickname() + " PRIVMSG " + (*it)->_incomingMsgs[1] + " " + msg + "\r\n";
										send((*itReceiver)->getFd(), resendMsg.c_str(), resendMsg.length(), 0);
									}
								}
								break;
							}
							case QUIT:
							{
								std::string quit = "QUIT\r\n";
								send((*it)->getFd(), quit.c_str(), quit.length(), 0);
								std::cout << "QUIT has been sent to " << (*it)->getNickname() << std::endl;
								removeUser(_users, (*it)->getFd());
								break;
							}
							default:
							{
								std::cout << "<SERVER`S DATA>"
										  << "\n";
								std::cout << "NickName - " << (*it)->getNickname() << "\n";
								std::cout << "UserName - " << (*it)->getUsername() << "\n";
								std::cout << "Server Name - " << _serverName << "\n";
							}
							}
						}
					}
				}
			}
			else if ((_fds[i].revents & POLLHUP) == POLLHUP)
			{
				std::vector<User *>::iterator it = std::find_if(_users.begin(), _users.end(), FindByFD(_fds[i].fd));
				removeUser(_users, (*it)->getFd());
				break;
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
	serverAddr.sin_addr.s_addr = INADDR_ANY;				   //(0.0. 0.0) any address for binding
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
	std::memset(&clientAddr, 0, sizeof(clientAddr));
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

void Server::removeUser(std::vector<User *> &users, int fd)
{
	// Удаление пользователя из списка пользователей
	std::vector<User *>::iterator itUser = std::find_if(users.begin(), users.end(), FindByFD(fd));
	if (itUser != users.end())
	{
		(*itUser)->closeSocket();
		users.erase(itUser);
	}
	// Удаление файлового дескриптора из _fds
	std::vector<struct pollfd>::iterator itFd = std::find_if(_fds.begin(), _fds.end(), FindByFD(fd));
	if (itFd != _fds.end())
	{
		_fds.erase(itFd);
	}
	std::cout << GREEN << "User has been removed from the server!" << RESET << std::endl;
}

//====================================<SIGNALS && SHUTDOWN>====================================

// Обработчик для SIGINT
void Server::sigIntHandler(int signal)
{
	std::cout << YELLOW << "\nReceived SIGINT (Ctrl+C) signal: " << signal << RESET << std::endl;
	if (globalServerInstance)
	{
		globalServerInstance->shutdownServer();
	} // Вызов метода для корректного завершения работы сервера
}

// Обработчик для SIGTERM
void Server::sigTermHandler(int signal)
{
	std::cout << YELLOW << "\nReceived SIGTERM signal: " << signal << RESET << std::endl;
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
		globalServerInstance->_disconnect = true;
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
