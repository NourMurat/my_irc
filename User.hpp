#pragma once

#include <iostream>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <deque>
#include <sstream>

class User
{
	private:
		int							_fd;
		bool        				_isAuth;
		bool						_isOP;
		std::string 				_buffer;
		std::string 				_realname;
		std::string 				_nickname;
		std::string 				_username;

	public:
		std::deque<std::string>		_incomingMsgs;
		// std::deque<std::string>		outgoingmsg;
		// std::deque<std::string>		messageDeque;
		User(int fd);
		~User();

		int     					getFd() const;
		void        				closeSocket();

		std::string 				getNickname() const;
		std::string 				getUsername() const;
		bool 						getIsAuth() const;

		void 						setNickname(std::string nickname);
		void 						setUsername(std::string username);
		void 						setIsAuth(bool isAuth);

		void						addMessage(std::string msg);
		std::deque<std::string> 	getMessageDeque();
		bool						getIsOP() const;
		void						setIsOP(bool isOP);

		size_t						receiveMsg();
		void						splitAndProcess(const std::string& data);
		std::string					getBuffer();

		void						parse(std::string msg);
		bool operator==(const User& other) const { // if find_if is not in cpp 98, change it
			return (this->_fd == other._fd);
		}
};
