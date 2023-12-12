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
		bool        _isAuth;
		std::string _realname;
		std::string _nickname;
		std::string _username;
	public:
		std::vector<std::string>	incomingmsg;
		std::vector<std::string>	outgoingmsg;
		std::deque<std::string>		messageDeque;
		User(int fd);
		~User();

		int     				getFd() const;
		std::string 			getNickname() const;
		std::string 			getUsername() const;
		bool 					getIsAuth() const;
		bool					getIsOP() const;
		std::deque<std::string> getMessageDeque();
		void 					setNickname(std::string nickname);
		void 					setUsername(std::string username);
		void 					setIsAuth(bool isAuth);

		void					addMessage(std::string msg);
		void        			closeSocket();
		void					setIsOP(bool isOP);



		void	parse(std::string msg);
		bool operator==(const User& other) const { // if find_if is not in cpp 98, change it
			return (this->_fd == other._fd);
		}
};
