#ifndef SERVER_HPP
#define SERVER_HPP

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
#include <sys/socket.h>

#include "Irc.hpp"
#include "User.hpp"
#include "Channel.hpp"
#include <csignal>
#include <cstdlib> // exit(signal)

#include <netinet/in.h>

const int MAX_CLIENTS = 4096;

class User;
class Server; // to use static signal and shutdown functions
extern Server* globalServerInstance; // to use static signal and shutdown functions

enum e_commands
{
	NOTCOMMAND,
	PING,
	NICK,
	USER,
	MSG,
	PRIVMSG,
	JOIN,
	PART,
	INVITE,
	TOPIC,
	KICK,
	MODE,
	INFO,
	PASS,
	AUTH,
	QUIT
};

class Server
{
    private:
		std::string 		_serverName;
        std::string     	_password;
        int             	_port;
        bool           		_disconnect;

	public:
		Server();
		Server (const int& port, const std::string& password);
		~Server();

		void 				runServer();
		int 				createSocket();
		void				bindSocket(int sockfd);
		void				listenSocket(int sockfd);
		int					acceptConection(int sockfd);
		void				removeUser(std::vector<User *>& users, int fd);
		void				removeChannelFromServer(std::string name);

		std::string			getServerName() const;
		void				setServerName(std::string serverName);
		void				welcomeMsg(User *user);

		std::vector<pollfd>		_fds;
		std::vector<User *>		_users;
		std::vector<Channel *>	_channels;

		void 				execMessage(User *user);
		int 				checkDupNickname(std::vector<User *> users, std::string nickname);

		static void			sigIntHandler(int signal);
		static void			sigTermHandler(int signal);
		static void			shutdownServer();
		bool				authenticateUser(int i);
		Channel*			getChannel(std::string name);
		int					isCommand(std::string command);


};

struct FindByFD
{
	int fd;

    FindByFD(int fd) : fd(fd) { }
    bool operator()(const User *user) const {
        return user->getFd() == fd;
    }
    bool operator()(const struct pollfd& pfd) const {
        return pfd.fd == fd;
    }
};

struct FindByNickname
{
	std::string nick;

    FindByNickname(const std::string &nick) : nick(nick) { }
    bool operator()(const User *user) const {
        return user->getNickname() == nick;
    }
	bool operator()(const std::pair<const std::string, User*>& pair) const {
        return pair.second->getNickname() == nick;
    }
};

#endif