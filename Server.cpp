#include "Server.hpp"

Server *globalServerInstance = NULL; // to use static signal and shutdown functions

Server::Server() : _port(0), _disconnect(true)
{
	std::cout << GREEN << "Server Default Constructor Called!" << RESET << std::endl;
	globalServerInstance = this;
}

Server::Server(const int &port, const std::string &password) : _serverName(""), _password(password), _port(port), _disconnect(true)
{
	std::cout << GREEN << "Server Parameter Constructor has called!" << RESET << std::endl;
	globalServerInstance = this;
}

Server::~Server()
{
	std::cout << GREEN << "Server Destructor Called" << RESET << std::endl;
	shutdownServer();
}

//===============================<START>========================================================

std::string Server::getServerName() const
{
	return this->_serverName;
}

void Server::setServerName(std::string serverName)
{
	this->_serverName = serverName;
}

void Server::removeChannelFromServer(std::string name)
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
	welcomeMsg = ":IRC 001 " + user->getNickname() + "!" + user->getUsername() + "@" + user->getUserHost() + " :Welcome to the Internet Relay Network " + user->getNickname() + "\n";
	send(user->getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
	welcomeMsg = ":IRC 002 " + user->getNickname() + "!" + user->getUsername() + "@" + user->getUserHost() + " :Your host is " + _serverName + ", running version V1.0\n";
	send(user->getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
	welcomeMsg = ":IRC 003 " + user->getNickname() + "!" + user->getUsername() + "@" + user->getUserHost() + " :This server was created in December 2023\n";
	send(user->getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
	welcomeMsg = ":IRC 004 " + user->getNickname() + "!" + user->getUsername() + "@" + user->getUserHost() + " :<servername> <version> <available user modes> <available channel modes>\n";
	send(user->getFd(), welcomeMsg.c_str(), welcomeMsg.length(), 0);
}

int toUpper(int c)
{
	return std::toupper(static_cast<unsigned char>(c));
}

std::string toUpperCase(const std::string &str)
{
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
	if (command2 == "/PASS")
		return (PASS);
	if (command2 == "INFO" || command2 == "/INFO")
		return (INFO);
	if (command2 == "AUTH" || command2 == "/AUTH")
		return (AUTH);
	if (command2 == "KICK" || command2 == "/KICK")
		return (KICK);
	return (NOTCOMMAND);
}

bool Server::authenticateUser(int i)
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
					return false;
				}
			}
			if ((*it)->_incomingMsgs[i] == "NICK")
			{
				if (checkDupNickname(_users, (*it)->_incomingMsgs[i + 1]))
				{
					std::string error = "ERROR :Nickname is already in use\r\n";
					send((*it)->getFd(), error.c_str(), error.length(), 0);
					return false;
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
	return true;
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
			break ;
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
						break ;
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
					std::cout << GREEN << "LOG:: User has been created with class User FD:" << _users[i]->getFd() << RESET << std::endl; // debugging -> create class User
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
						std::cout << RED << "Client disconnected FD:" << _fds[i].fd << RESET << std::endl;
						removeUser(_users, _fds[i].fd);
						i--;
					}
					else
					{
						std::vector<User *>::iterator it = std::find_if(_users.begin(), _users.end(), FindByFD(_fds[i].fd));
						if ((*it)->getBuffer() == "\r\n" || (*it)->getBuffer() == "" || (*it)->getBuffer() == "\n")
							return;
						std::cout << BLUE << "Received message from client" << _fds[i].fd << ":\n"
								  << RESET << (*it)->getBuffer() << std::endl;

						if (!(*it)->getIsAuth())
						{
							if (!authenticateUser(i))
							{
								removeUser(_users, (*it)->getFd());
								i--;
								break ;
							}

							if (((*it)->getIsAuth() && !(*it)->getNickname().empty()) && (!(*it)->getUsername().empty()) &&
								(!(*it)->getUserHost().empty()))
							{
								std::cout << GREEN << "<<< AUTHORIZED SUCCESS!!! >>>\n";
								std::cout << "USER \t\t\t- FD:" << (*it)->getFd() << "\n";
								std::cout << "NickName \t\t- " << (*it)->getNickname() << "\n";
								std::cout << "UserName \t\t- " << (*it)->getUsername() << "\n";
								std::cout << "UserIP \t\t\t- " << (*it)->getUserIP() << "\n";
								std::cout << "UserHost \t\t- " << (*it)->getUserHost() << RESET << "\n";
							}
						}

						if (isCommand((*it)->_incomingMsgs[0]) != NOTCOMMAND || (*it)->_incomingMsgs[0] != "WHOIS" || (!((*it)->_incomingMsgs[0] == "MODE" && (*it)->_incomingMsgs[1] == "FT_irc_server")))
						{
							int i = isCommand((*it)->_incomingMsgs[0]);

							switch (i)
							{
							// [0] = NICK
							// [1] = nickName
							case NICK:
							{
								if ((*it)->_incomingMsgs.size() < 2)
								{
									(*it)->write("ERROR :No nickname given\r\n");
									break ;
								}
								if ((*it)->getIsAuth() == true)
								{
									(*it)->write("ERROR :Can't change nick once authorized\r\n");
									break ;
								}
								if (checkDupNickname(_users, (*it)->_incomingMsgs[1]))
								{
									std::string error = "ERROR :Nickname is already in use\r\n";
									send((*it)->getFd(), error.c_str(), error.length(), 0);
									break ;
								}
								(*it)->setNickname((*it)->_incomingMsgs[1]);
								std::string msg = ":Your nickname has been changed to " + (*it)->getNickname() + "\r\n";
								std::cout << "nickname has been set to " << (*it)->getNickname() << "\n";
								send((*it)->getFd(), msg.c_str(), msg.length(), 0);
								break ;
							}
							// [0] = USER
							// [1] = userName
							case USER:
							{
								if ((*it)->_incomingMsgs.size() < 2)
								{
									(*it)->write("ERROR :No username given\r\n");
									break ;
								}
								if ((*it)->getIsAuth() == true)
								{
									(*it)->write("ERROR :Can't change user once authorized\r\n");
									break ;
								}
								(*it)->setUsername((*it)->_incomingMsgs[1]);
								std::string msg = ":Your username has been changed to " + (*it)->getUsername() + "\r\n";
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
							// [0] = JOIN
							// [1] = channelName
							// [2] = password
							case JOIN:
							{
								if ((*it)->getIsAuth() == false)
								{
									(*it)->write("ERROR :You're not authorized\r\n");
									break ;
								}
								if ((*it)->_incomingMsgs.size() < 2)
									break ;
								if ((*it)->_incomingMsgs[1][0] != '#')
									(*it)->_incomingMsgs[1].insert(0, "#");
								bool channelExists = false;
								for (std::vector<Channel *>::iterator itChannel = _channels.begin(); itChannel != _channels.end(); ++itChannel)
								{
									if ((*itChannel)->getName() == (*it)->_incomingMsgs[1])
									{
										channelExists = true;
										std::cout << MAGENTA << "DEBUG:: existing channel\n" << RESET << std::endl;
										if ((*itChannel)->getLimit() != 0 && (*itChannel)->countUsers((*itChannel)) >= (*itChannel)->getLimit())
										{
											std::string msg = "ERROR :Channel is full\r\n";
											send((*it)->getFd(), msg.c_str(), msg.length(), 0);
											break ;
										}
										if ((*itChannel)->isMember((*it)) || (*itChannel)->isOperator((*it)) || (*itChannel)->isOwner((*it)))
										{
											std::string msg = "ERROR :You're already on that channel\r\n";
											send((*it)->getFd(), msg.c_str(), msg.length(), 0);
											break ;
										}
										std::string pass_check = (*itChannel)->getPass();
                                        if (!pass_check.empty())
                                        {
                                            if ((*it)->_incomingMsgs.size() < 3)
                                            {
                                                std::string msg = "ERROR :No password given\r\n";
                                                send((*it)->getFd(), msg.c_str(), msg.length(), 0);
                                                break ;
                                            }
                                            if ((*itChannel)->getPass() != (*it)->_incomingMsgs[2])
                                            {
                                                std::string msg = "ERROR :Wrong password\r\n";
                                                send((*it)->getFd(), msg.c_str(), msg.length(), 0);
                                                break ;
                                            }
                                            std::cout << MAGENTA << "DEBUG:: password check\n" << RESET << std::endl;
                                            std::cout << MAGENTA << (*itChannel)->getPass() << RESET << std::endl;
                                            std::string check = (*it)->_incomingMsgs[2];
                                            if (check.empty())
                                                break ;
                                            std::cout << "check = " << check << std::endl;
                                        }
										if ((*itChannel)->addMember((*it)) == 1)
											break ;
										std::string msg = std::string(":IRC 332 ") + (*it)->getNickname() + " " + (*itChannel)->getName() + " " + (*itChannel)->getTopic() + "\r\n";
										send((*it)->getFd(), msg.c_str(), msg.length(), 0);
										std::map<std::string, User *>::iterator itMember = (*itChannel)->members.find((*it)->getNickname());
										if (itMember != (*itChannel)->members.end())
										{
											std::string msg2 = ":" + (*it)->getNickname() + " JOIN " + (*itChannel)->getName() + "\r\n";
											(*itChannel)->broadcast(msg2);
											std::cout << MAGENTA << "DEBUG:: JOIN NEW MEMBER!\n" << RESET << std::endl;
										}
										break ;
									}
								}
								if (!channelExists)
								{
									Channel *newChannel = new Channel((*it)->_incomingMsgs[1], (*it));
									if (!newChannel)
										break ;
									std::cout << GREEN << "LOG:: Channel created - " << newChannel->getName() << RESET << "\n";
									_channels.push_back(newChannel);
									std::string msg = std::string(":IRC 332 ") + (*it)->getNickname() + " " + newChannel->getName() + " " + newChannel->getTopic() + "\r\n";
									send((*it)->getFd(), msg.c_str(), msg.length(), 0);
									if (newChannel->isOwner((*it)))
									{
										std::string msg2 = ":" + (*it)->getNickname() + " JOIN " + newChannel->getName() + " \r\n";
										send((*it)->getFd(), msg2.c_str(), msg2.length(), 0);
										std::cout << MAGENTA << "DEBUGG:: NEW CHAN! " << RESET << "\n";
									}
								}
								break ;
							}
							// [0] = MSG or PRIVMSG
							// [1] = nickName or channelName(#)
							// [2][3][4]... = message
							case MSG:
							case PRIVMSG:
							{
								if ((*it)->getIsAuth() == false)
								{
									(*it)->write("ERROR :You're not authorized\r\n");
									break ;
								}
								if ((*it)->_incomingMsgs.size() < 3)
								{
									(*it)->write("ERROR :Wrong number of arguments PRIVMSG\r\n");
									break ;
								}
								if ((*it)->_incomingMsgs[1][0] != '#')
								{
									std::cout << MAGENTA << "DEBUGG:: Ordinary MSG" << RESET << "\n";
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
									for (std::vector<Channel *>::iterator itChannel = _channels.begin(); itChannel != _channels.end(); ++itChannel)
									{
										if ((*itChannel)->getName() == chanName)
										{
											if ((*itChannel)->isMember((*it)) || (*itChannel)->isOperator((*it)) || (*itChannel)->isOwner((*it)))
											{
												std::cout << MAGENTA << "DEBUGG:: Channel MSG" << RESET << "\n";
												std::string chanMSG = (*it)->_incomingMsgs[2];
												for (unsigned int i = 3; i < (*it)->_incomingMsgs.size(); i++)
													chanMSG += " " + (*it)->_incomingMsgs[i];
												std::string msg = ":" + (*it)->getNickname() + " PRIVMSG " + (*itChannel)->getName() + " " + chanMSG + "\r\n";
												(*itChannel)->broadcast(msg, (*it));
												break ;
											}
											else
											{
												std::string error = "ERROR :You're not on that channel\r\n";
												send((*it)->getFd(), error.c_str(), error.length(), 0);
												break ;
											}
										}
									}
									break ;
								}
							}
							// [0] = PASS
							// [1] = password
							case PASS:
							{
								if ((*it)->_incomingMsgs.size() < 2)
								{
									(*it)->write("ERROR :No password given\r\n");
									break ;
								}
								if ((*it)->getIsAuth() == true)
								{
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
									if ((*it)->getPassword() == _password && !(*it)->getNickname().empty() && !(*it)->getUsername().empty())
									{
										std::string msg = "You have been authorized\r\n";
										send((*it)->getFd(), msg.c_str(), msg.length(), 0);
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
							// [0] = PART
							// [1] = channelName
							case PART:
							{
								if ((*it)->getIsAuth() == false)
								{
									(*it)->write("ERROR :You're not authorized\r\n");
									break ;
								}

								if ((*it)->_incomingMsgs.size() < 2)
								{
									(*it)->write("ERROR :No channel given\r\n");
									break ;
								}
								if ((*it)->_incomingMsgs[1][0] != '#')
									(*it)->_incomingMsgs[1].insert(0, "#");

								for (std::vector<Channel *>::iterator itChannel = _channels.begin(); itChannel != _channels.end(); ++itChannel)
								{
									if ((*itChannel)->getName() == (*it)->_incomingMsgs[1])
									{
										std::cout << MAGENTA << "DEBUGG:: channels:  " << (*itChannel)->getName() << " == " << (*it)->_incomingMsgs[1] << RESET << "\n";
										if ((*itChannel)->isMember((*it)) || (*itChannel)->isOperator((*it)) || (*itChannel)->isOwner((*it)))
										{
											std::cout << MAGENTA << "DEBUGG:: PART CHAN" << RESET << "\n";
											if ((*itChannel)->removeUserFromChannel((*it)) == 1)
												removeChannelFromServer((*itChannel)->getName());
											break ;
										}
										else
										{
											std::string error = "ERROR :You're not on that channel\r\n";
											send((*it)->getFd(), error.c_str(), error.length(), 0);
											break ;
										}
									}
								}
								break ;
							}
							// [0] = KICK
							// [1] = channelName
							// [2] = nickName target
							case KICK:
							{
								if ((*it)->getIsAuth() == false)
								{
									(*it)->write("ERROR :You're not authorized\r\n");
									break ;
								}

								if ((*it)->_incomingMsgs.size() != 3)
								{
									(*it)->write("ERROR :No channel or user given\r\n");
									break ;
								}
								if ((*it)->_incomingMsgs[1][0] != '#')
									(*it)->_incomingMsgs[1].insert(0, "#");
								for (std::vector<Channel *>::iterator itChannel = _channels.begin(); itChannel != _channels.end(); ++itChannel)
								{
									if ((*itChannel)->getName() == (*it)->_incomingMsgs[1])
									{
										if (!(*itChannel)->isOwner((*it)) && !(*itChannel)->isOperator((*it)))
										{
											(*it)->write("ERROR :You're not the Owner or Operator!\r\n");
											break ;
										}
										else if ((*itChannel)->isOwner((*it)) || (*itChannel)->isOperator((*it)))
										{
											if ((*it)->getNickname() == (*it)->_incomingMsgs[2])
											{
												std::string msg = "ERROR :Can't kick yourself\r\n";
												send((*it)->getFd(), msg.c_str(), msg.length(), 0);
												break ;
											}
											std::map<std::string, User*>::iterator foundOp = (*itChannel)->operators.find((*it)->_incomingMsgs[2]);
											if (foundOp != (*itChannel)->operators.end())
											{
												std::string msg = ":" + (*it)->getNickname() + " KICK " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
												(*itChannel)->broadcast(msg);
												(*itChannel)->removeUserFromChannel((foundOp->second));
												break ;
											}
											std::map<std::string, User*>::iterator foundMember = (*itChannel)->members.find((*it)->_incomingMsgs[2]);
											if (foundMember != (*itChannel)->members.end())
											{
												std::string msg = ":" + (*it)->getNickname() + " KICK " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
												(*itChannel)->broadcast(msg);

												(*itChannel)->removeUserFromChannel((foundMember->second));
												break ;
											}
											if (((*itChannel)->getOwner())->getNickname() == (*it)->_incomingMsgs[2])
											{
												std::string msg = "ERROR :Nobody can kick the OWNER of the channel!\r\n";
												send((*it)->getFd(), msg.c_str(), msg.length(), 0);
												break ;
											}
											if (foundOp == (*itChannel)->operators.end() && foundMember == (*itChannel)->members.end()) 
											{
												std::string msg = "ERROR :User is not on that channel\r\n";
												send((*it)->getFd(), msg.c_str(), msg.length(), 0);
												break ;
											}
										}
										else
										{
											std::string error = "ERROR :You're not on that channel\r\n";
											send((*it)->getFd(), error.c_str(), error.length(), 0);
											break ;
										}
									}
								}
								break ;
							}
							// [0] = TOPIC
							// [1] = channelName
							// [2] = topic
							case TOPIC:
							{
								if ((*it)->getIsAuth() == false)
								{
									std::string error = "ERROR :You are not authorized\r\n";
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
										if ((*it)->_incomingMsgs.size() < 3)
										{
											std::string error = "ERROR :No channel or topic\r\n";
											send((*it)->getFd(), error.c_str(), error.length(), 0);
											break ;
										}
										userIsInChannel = true;
										if (((*itChannel)->isOperator((*it)) || (*itChannel)->isOwner((*it))) 
												&& (((*itChannel)->hasTopicRestrictions() == true) && (!(*it)->_incomingMsgs[2].empty())))
										{
											std::cout << MAGENTA << "DEBUGG:: TOPIC ONLY OPERATORS" << RESET << "\n";
											(*itChannel)->setTopic((*it)->_incomingMsgs[2]);
											std::string msg = ":" + (*it)->getNickname() + " TOPIC " + (*itChannel)->getName() + " " + (*itChannel)->getTopic() + "\r\n";
											(*itChannel)->broadcast(msg);
											break ;
										}
										else if ((*itChannel)->isMember((*it)) && (((*itChannel)->hasTopicRestrictions() == true) && (!(*it)->_incomingMsgs[2].empty())))
										{
											std::string error = "ERROR :A restriction has been set! \nONLY OPERATORS and channel OWNER can set TOPIC!\r\n";
											send((*it)->getFd(), error.c_str(), error.length(), 0);
											break ;
										}

										if (((*itChannel)->isOperator((*it)) || (*itChannel)->isOwner((*it)) || (*itChannel)->isMember(*it)) 
												&& ((*itChannel)->hasTopicRestrictions() == false) && (!(*it)->_incomingMsgs[2].empty()))
										{
											std::cout << MAGENTA << "DEBUGG:: TOPIC EVERYONE" << RESET << "\n";
											(*itChannel)->setTopic((*it)->_incomingMsgs[2]);
											std::string msg = ":" + (*it)->getNickname() + " TOPIC " + (*itChannel)->getName() + " " + (*itChannel)->getTopic() + "\r\n";
											(*itChannel)->broadcast(msg);
											break ;
										}

										break ;
									}
								}
								if (!userIsInChannel)
								{
									std::string error = "ERROR :You're not on that channel\r\n";
									send((*it)->getFd(), error.c_str(), error.length(), 0);
									break ;
								}
								break ;
							}
							// [0] = INVITE
							// [1] = nickName target
							// [2] = channelName
							case INVITE:
							{
								if ((*it)->getIsAuth() == false)
								{
									(*it)->write("ERROR :You're not authorized\r\n");
									break ;
								}
								if ((*it)->_incomingMsgs.size() != 3)
								{
									(*it)->write("ERROR :Wrong numbers of arguments (INVITE)\r\n");
									break ;
								}
								if ((*it)->_incomingMsgs[2][0] != '#')
									(*it)->_incomingMsgs[2].insert(0, "#");
								bool userIsInChannel = false;
								bool targetinServer = false;
								for (std::vector<Channel *>::iterator itChannel = _channels.begin(); itChannel != _channels.end(); ++itChannel)
								{
									if ((*itChannel)->getName() == (*it)->_incomingMsgs[2])
									{
										userIsInChannel = true;
										if (((*itChannel)->isOperator((*it)) || (*itChannel)->isOwner((*it))))
										{
											for (std::vector<User *>::iterator itUser = _users.begin(); itUser != _users.end(); ++itUser)
											{
												if ((itUser != _users.end()))
												{
													if ((*it)->getNickname() == (*it)->_incomingMsgs[1])
													{
														(*it)->write("ERROR :Can't invite yourself\r\n");
														break ;
													}
													if ((*itUser)->getNickname() == (*it)->_incomingMsgs[1])
													{
														targetinServer = true;
														std::string msg = ":" + (*it)->getNickname() + " INVITE " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
														send((*it)->getFd(), msg.c_str(), msg.length(), 0);
														(*itChannel)->addInvited(*itUser);
														std::map<std::string, User *>::iterator itFound = (*itChannel)->invited.find((*itUser)->getNickname());
														if (itFound != (*itChannel)->invited.end())
														{
															(*itFound->second).write(msg);
															std::cout << GREEN << "LOG:: " << (*it)->getNickname() << " invited " << itFound->first \
																<< " in the channel " << (*itChannel)->getName() << RESET << "\n";
															break ;
														}
														else if (itFound == (*itChannel)->invited.end())
														{
															std::string error = "ERROR :User has not been added to the invite list!\r\n";
															send((*it)->getFd(), error.c_str(), error.length(), 0);
															break ;
														}
													}
												}
											}
											if (!(targetinServer))
											{
												(*it)->write("ERROR :User is not connected\r\n");
												break ;
											}
										}
										else if ((!(*itChannel)->isOperator((*it)) && !(*itChannel)->isOwner((*it))) || (*itChannel)->isMember((*it)))
										{
											(*it)->write("ERROR :You're not the channel Owner or Operator\r\n");
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
									if ((*itChannel)->getName() != (*it)->_incomingMsgs[2])
									{
										std::string error = "ERROR :Channel does not exist\r\n";
										send((*it)->getFd(), error.c_str(), error.length(), 0);
										break ;
									}
								}
							}

							case MODE:
							{
								if ((*it)->getIsAuth() == false)
								{
									(*it)->write("ERROR :You're not authorized\r\n");
									break ;
								}
									if ((*it)->_incomingMsgs.size() < 3)
									{
										std::string error = "ERROR :Wrong numbers of arguments (MODE)\r\n";
										send((*it)->getFd(), error.c_str(), error.length(), 0);
										break ;
									}
								std::cout << MAGENTA << "DEBUGG:: check second arg --> " << (*it)->_incomingMsgs[1] << RESET << "\n";
								if ((*it)->_incomingMsgs[1][0] == '#')
								{
									for (std::vector<Channel *>::iterator itChannel = _channels.begin(); itChannel != _channels.end(); ++itChannel)
									{
										if ((*itChannel)->getName() == (*it)->_incomingMsgs[1]) // если такой канал существует
										{
											if ((*itChannel)->isOwner((*it)) || (*itChannel)->isOperator((*it))) // если ты оператор или владелец канала
											{
												if ((*it)->_incomingMsgs.size() < 3)
												{
													std::string error = "ERROR :Wrong numbers of arguments (MODE)\r\n";
													send((*it)->getFd(), error.c_str(), error.length(), 0);
													break ;
												}
												char sign = (*it)->_incomingMsgs[2][0];
												char mode = (*it)->_incomingMsgs[2][1];
												switch (mode)
												{
													// [0] = MODE
													// [1] = channelName
													// [2] = (+o) or (-o)
													// [3] = nickName target
													case 'o':
													{
														if ((*it)->_incomingMsgs.size() != 4)// в режиме оператора должно быть 4 аргумента
														{
															std::string error = "ERROR :Use 4 arguments to modify the operator mode (MODE o)\r\n";
															send((*it)->getFd(), error.c_str(), error.length(), 0);
															break ;
														}
														if (sign == '+')
														{
															std::map<std::string, User*>::iterator foundOp = (*itChannel)->operators.find((*it)->_incomingMsgs[3]);
															if (foundOp != (*itChannel)->operators.end())
															{
																std::string error = "ERROR :Such an operator already exists (MODE)\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															std::map<std::string, User*>::iterator itMembber = (*itChannel)->members.find((*it)->_incomingMsgs[3]);
															if (itMembber != (*itChannel)->members.end())
															{
																std::string msg = ":" + (*it)->getNickname() + " MODE " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
																send((*it)->getFd(), msg.c_str(), msg.length(), 0);
																std::cout << GREEN << "LOG:: (" << (*it)->getNickname()  << ") GAVE (" << itMembber->first << ") the channel operator privilege"  << RESET << "\n";
																(*itChannel)->addOperator(itMembber->second,(*it));
																break ;
															}
															else
															{
																std::string error = "ERROR :Such a member does not exist in the channel (MODE +o)\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															std::cout << MAGENTA << "DEBUGG:: MODE CHAN +o" << RESET << "\n";
															break ;
														}
														else if (sign == '-')
														{
															std::map<std::string, User *>::iterator foundOp = (*itChannel)->operators.find((*it)->_incomingMsgs[3]);
															if (foundOp == (*itChannel)->operators.end())
															{
																std::string error = "ERROR :Such an operator does not exist (MODE -o)\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															else
															{
																std::string msg = ":" + (*it)->getNickname() + " MODE " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
																send((*it)->getFd(), msg.c_str(), msg.length(), 0);
																std::cout << GREEN << "LOG:: (" << (*it)->getNickname() << ") TOOK channel operator privilege away from (" << foundOp->first << RESET << ")\n";
																(*itChannel)->takeOperatorPrivilege(foundOp->second);
																break ;
															}
															std::cout << MAGENTA << "DEBUGG:: MODE CHAN -o" << RESET << "\n";
															break ;
														}
														else
														{
															std::string error = "ERROR :Wrong sign (MODE o)\r\n";
															send((*it)->getFd(), error.c_str(), error.length(), 0);
															break ;
														}
													}
													// [0] = MODE
													// [1] = channelName
													// [2] = (+t) or (-t)
													case 't':
													{
														if ((*it)->_incomingMsgs.size() != 3)
														{
															std::string error = "ERROR :Wrong numbers of arguments (MODE t)!\r\n";
															send((*it)->getFd(), error.c_str(), error.length(), 0);
															break ;
														}
														if (sign == '+')
														{
															if ((*itChannel)->hasTopicRestrictions())
															{
																std::string error = "ERROR :Channel is already TOPIC RESTRICTED (MODE +t)\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															(*itChannel)->setTopicRestrictions(true);
															std::string msg = ":" + (*it)->getNickname() + " MODE " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
															send((*it)->getFd(), msg.c_str(), msg.length(), 0);
															std::string channelMsg = "NOTICE " + (*itChannel)->getName() + " :Now ONLY OPERATORS can change the channel TOPIC!\r\n";
															(*itChannel)->broadcast(channelMsg);
															break ;
														}
														else if (sign == '-')
														{
															if (!(*itChannel)->hasTopicRestrictions())
															{
																std::string error = "ERROR :Channel is already NOT TOPIC RESTRICTED (MODE -t)\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															(*itChannel)->setTopicRestrictions(false);
															std::string msg = ":" + (*it)->getNickname() + " MODE " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
															send((*it)->getFd(), msg.c_str(), msg.length(), 0);
															std::string channelMsg = "NOTICE " + (*itChannel)->getName() + " :Now EVERYONE can change the TOPIC of the channel!\r\n";
															(*itChannel)->broadcast(channelMsg);
															break ;
														}
														else
														{
															std::string error = "ERROR :Wrong sign (MODE t)\r\n";
															send((*it)->getFd(), error.c_str(), error.length(), 0);
															break ;
														}
													}
													// [0] = MODE
													// [1] = channelName
													// [2] = (+i) or (-i)
													case 'i':
													{
														if ((*it)->_incomingMsgs.size() != 3)
														{
															std::string error = "ERROR :Wrong numbers of arguments (MODE i)!\r\n";
															send((*it)->getFd(), error.c_str(), error.length(), 0);
															break ;
														}
														if (sign == '+')
														{
															if ((*itChannel)->isInviteOnly())
															{
																std::string error = "ERROR :Channel is already INVITE ONLY (MODE i)\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															(*itChannel)->setInviteOnly(true);
															std::string msg = ":" + (*it)->getNickname() + " MODE " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
															send((*it)->getFd(), msg.c_str(), msg.length(), 0);
															std::string channelMsg = "NOTICE " + (*itChannel)->getName() + " :Channel is now INVITE ONLY!\r\n";
															(*itChannel)->broadcast(channelMsg);
															break ;
														}
														else if (sign == '-')
														{
															if (!(*itChannel)->isInviteOnly())
															{
																std::string error = "ERROR :Channel is already NOT INVITE ONLY (MODE i)\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															(*itChannel)->setInviteOnly(false);
															std::string msg = ":" + (*it)->getNickname() + " MODE " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
															send((*it)->getFd(), msg.c_str(), msg.length(), 0);
															std::string channelMsg = "NOTICE " + (*itChannel)->getName() + " :Channel is NO longer INVITE ONLY!\r\n";
															(*itChannel)->broadcast(channelMsg);
															break ;
														}
														else
														{
															std::string error = "ERROR :Wrong sign (MODE i)\r\n";
															send((*it)->getFd(), error.c_str(), error.length(), 0);
															break ;
														}
													}
													// [0] = MODE
													// [1] = channelName
													// [2] = (+l) or (-l)
													// [3] = limit
													case 'l':
													{
														if (sign == '+')
														{
															if ((*it)->_incomingMsgs.size() != 4)
															{
																std::string error = "ERROR :Wrong numbers of arguments (MODE +l)!\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															if ((*itChannel)->hasUserLimit())
															{
																std::string error = "ERROR :Channel is already LIMITED (MODE +l)\nIf you want to change the limit, first turn off the limit mode and set a new limit!\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															(*itChannel)->setLimit(std::stoi((*it)->_incomingMsgs[3]));
															std::string msg = ":" + (*it)->getNickname() + " MODE " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
															send((*it)->getFd(), msg.c_str(), msg.length(), 0);
															std::string channelMsg = "NOTICE " + (*itChannel)->getName() + " :Channel is now LIMITED to " + (*it)->_incomingMsgs[3] + " users!\r\n";
															(*itChannel)->broadcast(channelMsg);
															break ;
														}
														else if (sign == '-')
														{
															if ((*it)->_incomingMsgs.size() != 3)
															{
																std::string error = "ERROR :Wrong numbers of arguments (MODE -l)!\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															if (!(*itChannel)->hasUserLimit())
															{
																std::string error = "ERROR :Channel is already NOT LIMITED (MODE -l)\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															(*itChannel)->removeUserLimit();
															std::string msg = ":" + (*it)->getNickname() + " MODE " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
															send((*it)->getFd(), msg.c_str(), msg.length(), 0);
															std::string channelMsg = "NOTICE " + (*itChannel)->getName() + " :Channel is NO longer LIMITED!\r\n";
															(*itChannel)->broadcast(channelMsg);
															break ;
														}
														else
														{
															std::string error = "ERROR :Wrong sign (MODE l)\r\n";
															send((*it)->getFd(), error.c_str(), error.length(), 0);
															break ;
														}
													}
													// [0] = MODE
													// [1] = channelName
													// [2] = (+k) or (-k)
													// [3] = password
													case 'k':
													{
														if (sign == '+')
														{
															if ((*it)->_incomingMsgs.size() != 4)
															{
																std::string error = "ERROR :Wrong numbers of arguments (MODE +k)!\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															(*itChannel)->setPass((*it)->_incomingMsgs[3]);
															std::string msg = ":" + (*it)->getNickname() + " MODE " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
															send((*it)->getFd(), msg.c_str(), msg.length(), 0);
															std::string channelMsg = "NOTICE " + (*itChannel)->getName() + " :Channel is now PASSWORD PROTECTED with password --> " + (*itChannel)->getPass() + "\r\n";
															(*itChannel)->broadcast(channelMsg);
															break ;
														}
														else if (sign == '-')
														{
															if ((*it)->_incomingMsgs.size() != 3)
															{
																std::string error = "ERROR :Wrong numbers of arguments (MODE -k)!\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															if ((*itChannel)->getPass().empty())
															{
																std::string error = "ERROR :Channel password is not set (MODE k)\r\n";
																send((*it)->getFd(), error.c_str(), error.length(), 0);
																break ;
															}
															else
															{
																(*itChannel)->setPass("");
																std::string msg = ":" + (*it)->getNickname() + " MODE " + (*itChannel)->getName() + " " + (*it)->_incomingMsgs[2] + "\r\n";
																send((*it)->getFd(), msg.c_str(), msg.length(), 0);
																std::string channelMsg = "NOTICE " + (*itChannel)->getName() + " :Channel is NO longer PASSWORD PROTECTED!\r\n";
																(*itChannel)->broadcast(channelMsg);
																break ;
															}
														}
														else
														{
															std::string error = "ERROR :Wrong sign (MODE k)\r\n";
															send((*it)->getFd(), error.c_str(), error.length(), 0);
															break ;
														}
													}
												}
											}
											else
											{
												std::string error = "ERROR :You're not an Operator or Owner of that channel (MODE)\r\n";
												send((*it)->getFd(), error.c_str(), error.length(), 0);
												break ;
											}
											break ;
										}
										else
										{
											std::string error = "ERROR :No such channel (MODE)\r\n";
											send((*it)->getFd(), error.c_str(), error.length(), 0);
											break ;
										}
									}
								}
								break ;
							}
							default:
							{
								break ;
							}
							}
						}
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

	char clientIP[INET6_ADDRSTRLEN];
	char clientHost[NI_MAXHOST];

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
	// Удаление файлового дескриптора из _fds
	std::vector<struct pollfd>::iterator itFd = std::find_if(_fds.begin(), _fds.end(), FindByFD(fd));
	if (itFd != _fds.end())
	{
		_fds.erase(itFd);
		std::cout << GREEN << "User has been removed from the server!" << RESET << std::endl;
	}
	// Удаление пользователя из списка пользователей
	std::vector<User *>::iterator itUser = std::find_if(users.begin(), users.end(), FindByFD(fd));
	if (itUser != users.end())
	{
		(*itUser)->closeSocket();
		users.erase(itUser);
	}
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
	exit(signal);
}

// Обработчик для SIGTERM
void Server::sigTermHandler(int signal)
{
	std::cout << YELLOW << "\nReceived SIGTERM signal: " << signal << RESET << std::endl;
	if (globalServerInstance)
	{
		globalServerInstance->shutdownServer();
	} // Вызов метода для корректного завершения работы сервера
	exit(signal);
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
