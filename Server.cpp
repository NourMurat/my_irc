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

// Channel Server::getChannel(std::string name)
// {
// 	for (std::vector<Channel *>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it)
// 	{
// 		if ((*it)->getName() == name)
// 			return (*it);
// 	}
// 	return NULL;
// }

void	Server::removeChannelFromServer(std::string name)
{
	for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		if ((*it)->getName() == name)
		{
			_channels.erase(it);
			break ;
		}
	}
}

int Server::checkDupNickname(std::vector<User *> users, std::string nickname)
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
	welcomeMsg = ":IRC 001 " + user->getNickname() + " :Welcome to the Internet Relay Network " + user->getNickname() + "\n";
	send(user->getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
	welcomeMsg = ":IRC 002 " + user->getNickname() + " :Your host is " + _serverName + ", running version V1.0\n";
	send(user->getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
	welcomeMsg = ":IRC 003 " + user->getNickname() + " :This server was created in December 2023\n";
	send(user->getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
	welcomeMsg = ":IRC 004 " + user->getNickname() + " :<servername> <version> <available user modes> <available channel modes>\n";
	send(user->getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
}

int toUpper(int c) {
    return std::toupper(static_cast<unsigned char>(c));
}

std::string toUpperCase(const std::string& str) {
    std::string upperCaseStr = str;
    std::transform(upperCaseStr.begin(), upperCaseStr.end(), upperCaseStr.begin(), toUpper);
    return upperCaseStr;
}


int Server::isCommand(std::string command)
{
	std::string command2 = toUpperCase(command);

	if (command2 == "NICK" || command2 == "/NICK")
		return (NICK);
	if (command2 == "USER" || command2 == "/USER")
		return (USER);
	if (command2 == "JOIN" || command2 == "/JOIN")
		return (JOIN);
	if (command2 == "MSG" || command2 == "/MSG")
		return (MSG);
	if (command2 == "PRIVMSG" || command2 == "/PRIVMSG")
		return (PRIVMSG);
	if (command2 == "PING" || command2 == "/PING")
		return (PING);
	if (command2 == "PART" || command2 == "/PART")
		return (PART);
	if (command2 == "INVITE" || command2 == "/INVITE")
		return (INVITE);
	if (command2 == "TOPIC" || command2 == "/TOPIC")
		return (TOPIC);
	if (command2 == "MODE" || command2 == "/MODE")
		return (MODE);
	if (command2 == "QUIT" || command2 == "/QUIT")
		return (QUIT);
	if (command2 == "PASS" || command2 == "/PASS")
		return (PASS);
	if (command2 == "INFO" || command2 == "/INFO")
		return (INFO);
	if (command2 == "AUTH" || command2 == "/AUTH")
		return (AUTH);
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
				setServerName((*it)->_incomingMsgs[i + 3]); // server name
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
		signal(SIGINT, Server::sigIntHandler);
		signal(SIGTERM, Server::sigTermHandler);
		// int poll(representing a FD, number of FD, timeout);
		int numFds = poll(_fds.data(), _fds.size(), -1);
		if (numFds == -1)
		{
			throw std::runtime_error("ERROR! Poll error!\n");
		}
		for (int i = 0; i < (int)_fds.size(); i++)
		{
			if (_fds[i].revents & POLLOUT && _fds[i].fd != sockfd)
			{
				User *user = NULL;
				for (std::vector<User *>::iterator it = _users.begin(); it != _users.end(); ++it)
				{
					if ((*it)->getFd() == _fds[i].fd)
					{
						user = *it;
						break;
					}
				}
				if (user != NULL)
				{
					if (!user->getOutgoingMsg().empty())
					{
						std::string message = user->getOutgoingMsg()[0];
						send(_fds[i].fd, message.c_str(), message.length(), 0);
					}
				}
			}

			if (_fds[i].revents & POLLIN)
			{ // data that can be read without blocking AND can safely read operation be on it
				if (_fds[i].fd == sockfd)
				{
					// New client connection and add it to "users, _fds" vectors
					int clientFd = acceptConection(sockfd);
					std::cout << MAGENTA << "DEBUG:: User has been created with class User FD:" << _users[i]->getFd() << RESET << std::endl; // debugging -> create class User
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
						// (*it)->parse((*it)->_incomingMsgs[0]); // Reem`s function
						// for (size_t i = 0; i < (*it)->_incomingMsgs.size(); ++i) //debugging, delete it before submit
						//     std::cout << i << ": " << (*it)->_incomingMsgs[i] << std::endl; //debugging, delete it before submit
						if (!(*it)->getIsAuth())
						{
							authenticateUser(i);
							//debugging, delete it before submit
							if (((*it)->getIsAuth() && !(*it)->getNickname().empty()) && (!(*it)->getUsername().empty()) && \
									(!(*it)->getUserHost().empty())) {
								std::cout << GREEN << "<<< AUTHORIZED SUCCESS!!! >>>\n";
								std::cout << "USER \t\t\t- FD:"<< (*it)->getFd() << "\n";
								std::cout << "NickName \t\t- "<< (*it)->getNickname() << "\n";
								std::cout << "UserName \t\t- "<< (*it)->getUsername() << "\n";
								std::cout << "UserIP \t\t\t- "<< (*it)->getUserIP() << "\n";
								std::cout << "UserHost \t\t- "<< (*it)->getUserHost() << RESET << "\n";
							}
						}
						if (isCommand((*it)->_incomingMsgs[0]) != NOTCOMMAND)
						{
							int i = isCommand((*it)->_incomingMsgs[0]);

							switch (i)
							{
								case NICK:
								{
									std::string check = (*it)->_incomingMsgs[1];
									if (check.empty())
									{
										(*it)->write("ERROR :No nick given\r\n");
										break ;
									}
									if (checkDupNickname(_users, (*it)->_incomingMsgs[1]))
									{
										std::string error = "ERROR :Nickname is already in use\r\n";
										send((*it)->getFd(), error.c_str(), error.length(), 0);
										break ;
									}
									(*it)->setNickname((*it)->_incomingMsgs[1]);
									std::string msg = "Your nickname has been changed to " + (*it)->getNickname() + "\r\n";
									std::cout << "nickname has been set to "<< (*it)->getNickname() << "\n";
									send((*it)->getFd(), msg.c_str(), msg.length(), 0);
									break ;
								}
								case USER:
								{
									std::string check = (*it)->_incomingMsgs[1];
									if (check.empty())
									{
										(*it)->write("ERROR :No username given\r\n");
										break ;
									}
									(*it)->setUsername((*it)->_incomingMsgs[1]);
									std::string msg = "Your username has been changed to " + (*it)->getNickname() + "\r\n";
									std::cout << "username has been set to " << (*it)->getUsername() << std::endl;
									send((*it)->getFd(), msg.c_str(), msg.length(), 0);
									break ;
								}
								case PING:
								{
									std::string pong = "PONG\r\n";
									send((*it)->getFd(), pong.c_str(), pong.length(), 0);
									std::cout << "PONG has been sent to " << (*it)->getNickname() << std::endl;

									std::cout << "\nSERVER`S DATA:" << "\n";
									std::cout << "Server`s PORT \t\t" << _port << "\n";
									std::cout << "Server`s PASSWORD \t" << _password << "\n";
									std::cout << "Server`s NAME \t\t" << _serverName << "\n\n";
									std::cout << "UserFD \t\t\t" << (*it)->getFd() << "\n";
									std::cout << "NickName \t\t" << (*it)->getNickname() << "\n";
									std::cout << "UserName \t\t" << (*it)->getUsername() << "\n";
									std::cout << "UserIP \t\t\t" << (*it)->getUserIP() << "\n";
									std::cout << "UserHost \t\t" << (*it)->getUserHost() << "\n";
									break ;
								}
								case QUIT:
								{
									std::string quit = "QUIT\r\n";
									send((*it)->getFd(), quit.c_str(), quit.length(), 0);
									std::cout << "QUIT has been sent to " << (*it)->getNickname() << std::endl;
									removeUser(_users, (*it)->getFd());
									break ;
								}
								case JOIN:
								{
									if ((*it)->getIsAuth() == false)
										break;
									// Channel *newChannel = new Channel((*it)->_incomingMsgs[1], (*it));
									// if(messages.size() < 2)
									// 	return;
									// size_t x = 0;
									// if ((*it)->_incomingMsgs[1][0] != '#')
									// 	(*it)->_incomingMsgs[1].insert(0, "#");
									// newChannel = irc::Server::serverInstance->getChannel(user->_channelToJoin.at(x));
									// join_channel(user->_channelToJoin.at(x), user, channel, "");
									// x++;


									if ((*it)->_incomingMsgs.size() < 2)
										break ;
									if ((*it)->_incomingMsgs[1][0] != '#')
										(*it)->_incomingMsgs[1].insert(0, "#");
									bool channelExists = false;
									for (std::vector<Channel *>::iterator itChannel = _channels.begin(); itChannel != _channels.end(); ++itChannel)
									{
										if ((*itChannel)->getName() == (*it)->_incomingMsgs[1])
										{
											std::cout << MAGENTA << "DEBUG:: existing channel\n" << RESET << std::endl;
											(*itChannel)->addMember((*it));
											std::string msg = ":" + (*it)->getNickname() + " JOIN " + (*itChannel)->getName() + "\r\n";
											(*itChannel)->broadcast(msg);
											channelExists = true;
											break;
										}
									}
									if (!channelExists)
									{
										std::cout << MAGENTA << "DEBUGG:: New CHAN" << RESET << "\n";
										Channel *newChannel = new Channel((*it)->_incomingMsgs[1], (*it));
										std::cout << MAGENTA << "DEBUGG:: Channel created - " << newChannel->getName() << RESET << "\n";
										if (!newChannel)
											break ;
										_channels.push_back(newChannel);
										// newChannel->addMember(*it);
										std::string msg = ":IRC 332 " + (*it)->getNickname() + " " + newChannel->getName() + " " + newChannel->getTopic() + "\r\n";
										send((*it)->getFd(), msg.c_str(), msg.length(), 0);
										// for (size_t i = 0; i < newChannel->members.size() ; ++i)
										// {
										// 	std::string msg2 = ":" + (*it)->getNickname() + " JOIN " + newChannel->getName() + " \r\n";
										// 	send(newChannel->members.at((*it)->getNickname())->getFd(), msg2.c_str(), msg2.length(), 0);
										// }

										std::map<std::string, User*>::iterator found = newChannel->members.find((*it)->getNickname());
										if (found != newChannel->members.end()) {
    									// Элемент найден, 'found->second' указывает на объект User
    										std::string msg2 = ":" + (*it)->getNickname() + " JOIN " + newChannel->getName() + " \r\n";
    										send(found->second->getFd(), msg2.c_str(), msg2.length(), 0);
										}
									}
									break ;
								}
								case MSG:
								case PRIVMSG:
								{
									if ((*it)->getIsAuth() == false)
										break;
									if ((*it)->_incomingMsgs[1][0] != '#')
									{
										std::cout << MAGENTA << "DEBUGG:: PRIV" << RESET << "\n";
										std::vector<User *>::iterator itReceiver = std::find_if(_users.begin(), _users.end(), FindByNickname((*it)->_incomingMsgs[1]));
										if (itReceiver == _users.end())
											break ;
										std::string msg = (*it)->_incomingMsgs[2];
										if (msg.empty())
										{
											(*it)->write("ERROR :No nick given\r\n");
											break ;
										}
										if ((*itReceiver)->getFd() != -1)
										{
											std::cout << MAGENTA << "DEBUGG:: PRIV" << RESET << "\n";
											for (unsigned int index = 3; index < (*it)->_incomingMsgs.size(); ++index)
												msg += " " + (*it)->_incomingMsgs[index];
											std::string resendMsg = ":" + (*it)->getNickname() + " PRIVMSG " + (*it)->_incomingMsgs[1] + " " + msg + "\r\n";
											send((*itReceiver)->getFd(), resendMsg.c_str(), resendMsg.length(), 0);
										}
										break ;
									}
									else if ((*it)->_incomingMsgs[1][0] == '#')
									{
										std::string chanName = (*it)->_incomingMsgs[1];
										bool userIsInChannel = false;
										for (std::vector<Channel *>::iterator itChannel = _channels.begin(); itChannel != _channels.end(); ++itChannel)
										{
											if ((*itChannel)->getName() == chanName)
											{
												std::map<std::string, User*>::iterator itLiveUser = std::find_if((*itChannel)->members.begin(), (*itChannel)->members.end(), FindByNickname((*it)->getNickname()));
												if (itLiveUser != (*itChannel)->members.end())
												{
													std::cout << MAGENTA << "DEBUGG:: PRIV CHAN" << RESET << "\n";
													userIsInChannel = true;
													std::string chanMSG = (*it)->_incomingMsgs[2];
													for (unsigned int i = 3; i < (*it)->_incomingMsgs.size(); i++)
														chanMSG += " " + (*it)->_incomingMsgs[i];
													std::string msg = ":" + (*it)->getNickname() + " PRIVMSG " + (*itChannel)->getName() + " " + chanMSG + "\r\n";
													// send((*it)->getFd(), msg.c_str(), msg.length(), 0);
													(*itChannel)->broadcast(msg, (*it));
													break ;
												}
												else if (!userIsInChannel)
												{
													std::string error = "ERROR :You're not on that channel\r\n";
													send((*it)->getFd(), error.c_str(), error.length(), 0);
													break ;
													
												}
												break ;
											}
										}
										break ;
									}
								}
								case PASS:
								{	
									std::string check = (*it)->_incomingMsgs[1];
									if (check.empty())
									{
										(*it)->write("ERROR :No password given\r\n");
										break ;
									}
									if ((*it)->_incomingMsgs[1] != _password)
									{
										std::string error = "ERROR :Wrong password\r\n";
										send((*it)->getFd(), error.c_str(), error.length(), 0);
										(*it)->setPassword("");
										break ;
									}
									else
										(*it)->setPassword((*it)->_incomingMsgs[1]);
									break ;
								}
								case INFO:
								{
									std::string msg;

									if ((*it)->getNickname().empty())
										msg = "Your nickname is not set\r\n";
									else
										msg = "Your nickname is " + (*it)->getNickname() + "\r\n";
									send((*it)->getFd(), msg.c_str(), msg.length(), 0);
									if ((*it)->getUsername().empty())
										msg = "Your username is not set\r\n";
									else
										msg = "Your username is " + (*it)->getUsername() + "\r\n";
									send((*it)->getFd(), msg.c_str(), msg.length(), 0);
									if ((*it)->getPassword().empty())
										msg = "Your password is not set\r\n";
									else
										msg = "Your password is " + (*it)->getPassword() + "\r\n";
									send((*it)->getFd(), msg.c_str(), msg.length(), 0);
									break ;
								}
								case AUTH:
								{
									if ((*it)->getIsAuth())
									{
										std::string msg = "You are already authorized\r\n";
										send((*it)->getFd(), msg.c_str(), msg.length(), 0);
										break ;
									}
									else
									{
										if ((*it)->getPassword() == _password && !(*it)->getNickname().empty() && \
											!(*it)->getUsername().empty())
										{
											(*it)->write("You have been authorized\r\n");
											(*it)->setIsAuth(true);
										}
										else
										{
											std::string msg = "Error: please provide a password, nick, and username to be authorized\r\n";
											send((*it)->getFd(), msg.c_str(), msg.length(), 0);
											break ;
										}
									}
									break ;
								}
								case PART:
								{
									if ((*it)->getIsAuth() == false)
										break;
									if ((*it)->_incomingMsgs.size() < 2)
									{
										std::string error = "ERROR :No channel given\r\n";
										send((*it)->getFd(), error.c_str(), error.length(), 0);
										break ;
									}
									if ((*it)->_incomingMsgs[1][0] != '#')
										(*it)->_incomingMsgs[1].insert(0, "#");
									bool userIsInChannel = false;
									for (std::vector<Channel *>::iterator itChannel = _channels.begin(); itChannel != _channels.end(); ++itChannel)
									{
										if ((*itChannel)->getName() == (*it)->_incomingMsgs[1])
										{
												std::cout << MAGENTA << "DEBUGG:: PART CHAN" << RESET << "\n";
												userIsInChannel = true;
												std::string msg = ":" + (*it)->getNickname() + " PART " + (*itChannel)->getName() + "\r\n";
												send((*it)->getFd(), msg.c_str(), msg.length(), 0);
												if ((*itChannel)->removeUserFromChannel((*it)) == 1)
													removeChannelFromServer((*itChannel)->getName());
												break ;
											if (!userIsInChannel)
											{
												std::string error = "ERROR :You're not on that channel\r\n";
												send((*it)->getFd(), error.c_str(), error.length(), 0);
												break ;
												
											}
											break ;
										}
									}
									break ;
								}
								// case KICK:
								// {
								// 	if ((*it)->getIsAuth() == false)
								// 		break;
								// 	if ((*it)->_incomingMsgs.size() < 3)
								// 	{
								// 		std::string error = "ERROR :No channel or user given\r\n";
								// 		send((*it)->getFd(), error.c_str(), error.length(), 0);
								// 		break ;
								// 	}
								// 	if ((*it)->_incomingMsgs[1][0] != '#')
								// 		(*it)->_incomingMsgs[1].insert(0, "#");
								// 	bool userIsInChannel = false;
								// 	for (std::vector<Channel *>::iterator itChannel = _channels.begin(); itChannel != _channels.end(); ++itChannel)
								// 	{
								// 		if ((*itChannel)->getName() == (*it)->_incomingMsgs[1])
								// 		{
								// 			if ((!(*itChannel)->isOwner() && !(*itChannel)->isOperator()) || (*it)->_incomingMsgs[2]) == (*itChannel)->getOwner()->getNickname()){
								// 			// Only the channel owner can KICK a user from the channel
								// 			(*it)->write((*it)->getNickname() + " " + (*it)->_incomingMsgs[1] + 
								// 						" :You're not the channel Owner or Operator! .");
								// 			break ;
								// 			}
								// 			if ((*itChannel)->isOwner((*it)) || (*itChannel)->isOperator((*it)))
								// 			{
								// 				std::cout << MAGENTA << "DEBUGG:: KICK CHAN" << RESET << "\n";
								// 				userIsInChannel = true;
								// 				std::string msg = ":" + (*it)->getNickname() + " KICK " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
								// 				send((*it)->getFd(), msg.c_str(), msg.length(), 0);
								// 				(*itChannel)->removeMemberOrOperatorFromChannel((*it));
								// 				break ;
								// 			}
								// 			else if (!userIsInChannel)
								// 			{
								// 				std::string error = "ERROR :You're not on that channel\r\n";
								// 				send((*it)->getFd(), error.c_str(), error.length(), 0);
								// 				break ;
												
								// 			}
								// 			break ;
								// 		}
								// 	}
								// 	break ;
								// }
								default:
								{
									break;
								}
							}
						}
						// if (((*it)->getIsAuth() && !(*it)->getNickname().empty()) && (!(*it)->getUsername().empty()) && \
						// 		(!(*it)->getUserHost().empty())) {
						// 	execMessage((*it));
						// }
						
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
	struct sockaddr_storage clientAddr; // hold clientAddr information
	std::memset(&clientAddr, 0, sizeof(clientAddr));
	socklen_t clientLen = sizeof(clientAddr);
	// int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
	int clientFd = accept(sockfd, (struct sockaddr *)&clientAddr, &clientLen);
	if (clientFd == -1)
	{
		std::cerr << RED << "Failed to accept << " << RESET << std::endl;
		exit(EXIT_FAILURE);
	}

	char	clientIP[INET6_ADDRSTRLEN];
	char	clientHost[NI_MAXHOST];

	if (clientAddr.ss_family == AF_INET)
	{
		struct sockaddr_in *ipv4 = (struct sockaddr_in *)&clientAddr;
		inet_ntop(AF_INET, &(ipv4->sin_addr), clientIP, INET_ADDRSTRLEN);
	}
	else if (clientAddr.ss_family == AF_INET6)
	{
		struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&clientAddr;
		inet_ntop(AF_INET6, &(ipv6->sin6_addr), clientIP, INET6_ADDRSTRLEN);
	}
	else
	{
		std::cout << "Unknown address family" << std::endl;
		return -1;
	}

	int result = getnameinfo((struct sockaddr *)&clientAddr, clientLen, clientHost, NI_MAXHOST, NULL, 0, NI_NUMERICSERV);
    if (result != 0)
    {
        std::cerr << "Error getting hostname: " << gai_strerror(result) << std::endl;
        std::strcpy(clientHost, "Unknown");
    }
    std::cout << GREEN << "\nSuccessfully accepted connection from " << clientIP << " (Hostname: " << clientHost << ")" << RESET << std::endl;

	pollfd tmp2 = {clientFd, POLLIN, 0};
	_fds.push_back(tmp2);
	_users.push_back(new User(clientFd, clientIP, clientHost));

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
		// delete *itUser;
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
void	Server::sigIntHandler(int signal)
{
	std::cout << YELLOW << "\nReceived SIGINT (Ctrl+C) signal: " << signal << RESET << std::endl;
	if (globalServerInstance)
	{
		globalServerInstance->shutdownServer();
	} // Вызов метода для корректного завершения работы сервера
}

// Обработчик для SIGTERM
void	Server::sigTermHandler(int signal)
{
	std::cout << YELLOW << "\nReceived SIGTERM signal: " << signal << RESET << std::endl;
	if (globalServerInstance)
	{
		globalServerInstance->shutdownServer();
	} // Вызов метода для корректного завершения работы сервера
}

// Метод для корректного завершения работы сервера
void	Server::shutdownServer()
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

void	Server::execMessage(User *user)
{
	std::string message = user->_incomingMsgs[0];
	if (message == "PING")
	{
		std::string reply = "PONG\r\n";
		send(user->getFd(), reply.c_str(), reply.length(), 0);
		return;
	}
	else if (message == "PRIVMSG")
	{
		if (user->_incomingMsgs.size() < 3)
			return;
		std::string msg = user->_incomingMsgs[2];
		if (user->_incomingMsgs[1][0] != '#')
		{
			std::vector<User *>::iterator itReceiver = std::find_if(_users.begin(), _users.end(), FindByNickname(user->_incomingMsgs[1]));
			if((*itReceiver)->getFd() != -1)
            {
				msg = user->_incomingMsgs[2];
				for (unsigned int i = 3; i < user->_incomingMsgs.size(); i++)
					msg += " " + user->_incomingMsgs[i];
				std::string resendMsg = ":" + user->getNickname() + " PRIVMSG " + user->_incomingMsgs[1] + " " +  msg + "\r\n";
				std::cout << "sending: " << resendMsg << std::endl;
				send((*itReceiver)->getFd(), resendMsg.c_str(), resendMsg.length(), 0);
        	}
			// else
			// {
			// 	irc::Server::serverInstance->sendMsg(user->getUserFd(), ":COY 401, No such nick/channel\r\n");
			// }
		}
		// else if (messages[1][0] == '#')
		// {
		// 	std::vector<Channel *> channels = irc::Server::serverInstance->getChannels();
		// 	std::vector<Channel *>::iterator it;
		// 	std::cout << "channel name: " << messages[1] << std::endl;

		// 	for (it = channels.begin();  it != channels.end(); ++it)
		// 	{
		// 		if ((*it)->getName() == messages[1]){

		// 		std::cout << "channel nammme: " << (*it)->getName() << std::endl;
		// 			break;
		// 		}
		// 	}
		// 	if (it == channels.end())
		// 	{
		// 		irc::Server::serverInstance->sendMsg(user->getUserFd(), ":COY 401, No such nick/channel\r\n");
		// 		return;
		// 	}
		// 	channel = *it;
		// 	msg = messages[2];
		// 	std::vector<User *>::iterator it2;
		// 	for (it2 = channel->users.begin(); it2 != channel->users.end(); ++it2)
		// 	{
		// 		if((*it2)->getNickName() == user->getNickName())
		// 		{
		// 			std::cout << "found user" << std::endl;
		// 			break ;
		// 		}
		// 	}
		// 	if(it2 == channel->users.end())
		// 	{
		// 		irc::Server::serverInstance->sendMsg(user->getUserFd(), ":COY 401, User not in channel\r\n");
		// 		return;
		// 	}
		// 	for (unsigned int i = 3; i < messages.size(); i++)
		// 		msg += " " + messages[i];
		// 	std::string msg2 = ":" + user->getNickName() + " PRIVMSG " + messages[1] + " " +  msg + "\r\n";
		// 	channel->sendMessage(msg2, user->getNickName());
		// }
		return;
	}
	// else if (message == "NICK")
	// {
	// 	handle_nickname(user, messages);
	// }


	// if (isCommand(message) == false)
	// {
	// 	irc::Server::serverInstance->sendMsg(user->getUserFd(), ":COY 421, Unknown command\r\n");
	// 	return;
	// }
	// Channel *channel = irc::Server::serverInstance->getChannel(messages[1]);
	// if (message == "INVITE")
	// 	channel = irc::Server::serverInstance->getChannel(messages[2]);
	// if (server_messages(messages) == true)
	// 	return;
	// if (message == "JOIN")
	// {
	// 	if(messages.size() < 2)
	// 		return;
	// 	size_t x = 0;
	// 	while (x < user->_channelToJoin.size())
	// 	{
	// 		if(user->_channelToJoin.at(x).at(0) != '#')
	// 			user->_channelToJoin.at(x).insert(0, "#");
	// 		channel = irc::Server::serverInstance->getChannel(user->_channelToJoin.at(x));
	// 		if(user->_channelKeys.size())
	// 			join_channel(user->_channelToJoin.at(x), user, channel, user->_channelKeys.at(x));
	// 		else
	// 			join_channel(user->_channelToJoin.at(x), user, channel, "");
	// 		x++;
	// 	}
	// 	return;
	// }



	// if (channel == NULL)
	// {
	// 	irc::Server::serverInstance->sendMsg(user->getUserFd(), ":COY 442, You're not on that channel\r\n");
	// 	return;
	// }
	// if (message == "PART")
	// {
	// 	std::string msg = ":" + user->getNickName() + " PART :" + channel->getName() + "\r\n";
	// 	irc::Server::serverInstance->sendMsg(user->getUserFd()  , msg);
	// 	channel->sendMessage(":" + user->getNickName() + " PART " + channel->getName() + " :leaving", user->getNickName());
	// 	if(channel->partChannel(user) == true)
	// 	{
	// 		return ;
	// 	}
	// }
	// else if (message == "MODE")
	// {
	// 	channel->switchMode(user, messages);
	// }
	// else if (message == "TOPIC")
	// {
	// 	channel->execTopic(user, messages);
	// }
	// else if (message == "KICK" && user->is_op(*channel) == true)
	// {
	// 	channel->kickUser(channel, user, messages);
	// }
	// else if (message == "INVITE")
	// {
	// 	channel->inviteUser(user, messages);
	// }
}
