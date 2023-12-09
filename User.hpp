#pragma once

#include <iostream>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <deque>
class User
{
	private:
		int			_fd;
		std::string _nickname;
		std::string _username;
		bool        _isAuth;

	public:
		std::vector<std::string>	incomingmsg;
		std::vector<std::string>	outgoingmsg;
		std::deque<std::string>		messageDeque;
		User(int fd);
		~User();

		int     				getFd() const;
		void        			closeSocket();
		std::string 			getNickname() const;
		std::string 			getUsername() const;
		bool 					getIsAuth() const;
		void 					setNickname(std::string nickname);
		void 					setUsername(std::string username);
		void 					setIsAuth(bool isAuth);
		void					addMessage(std::string msg);
		std::deque<std::string> getMessageDeque();



		void	parse(std::string msg);
		bool operator==(const User& other) const { // if find_if is not in cpp 98, change it
			return (this->_fd == other._fd);
		}
};
