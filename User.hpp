#pragma once

#include <iostream>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <deque>
#include <sstream>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>

#include "Irc.hpp"
#include "Channel.hpp"

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
		std::string					_userIP;
		std::string					_userHost;
		std::string 				_password;
	public:
		User(int fd, std::string userIP, std::string userHost);
		~User();

		// Channels in which the client is a member
		std::map<std::string, Channel*> channelsOfClient;

		std::vector<std::string>		_incomingMsgs;
		std::vector<std::string>		_outgoingMsgs;

		void        				closeSocket();
		int     					getFd() const;

		std::vector<std::string> & 	getOutgoingMsg();
	    void 						setOutgoingMsg( std::string msg );
		void 						printOutgoingMsgs();

		std::string 				getNickname() const;
		std::string 				getUsername() const;
		std::string					getUserIP() const;
		std::string					getUserHost() const;
		std::string					getBuffer() const;
		bool 						getIsAuth() const;
		std::string 				getPassword() const;

		void        				setPassword(std::string password);
		void 						setNickname(std::string nickname);
		void 						setUsername(std::string username);
		void						setUserIP(std::string userIP);
		void						setUserHost(std::string userHost);
		void 						setIsAuth(bool isAuth);

		void						addMessage(std::string msg);
		bool						getIsOP() const;
		void						setIsOP(bool isOP);

		size_t						receiveMsg();
		void						splitAndProcess(const std::string& data);

		void						parse(std::string msg);
		bool operator==(const User& other) const { // if find_if is not in cpp 98, change it
			return (this->_fd == other._fd);
		}

		// Method for adding a client to a channel
		void						joinChannel(const std::string& channelName, Channel* channel);
		
		//Method for removing a client from a channel
		void						removeChannelOfClient(const std::string& channelName);
		
		// Method to get the list of channels the client is connected to
		const std::map<std::string, Channel*>&		getChannelsOfClient() const;
		
		// sends a message over an open socket
		void						write(const std::string& message) const;

};
