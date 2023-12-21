#pragma once

#include <stdio.h>
#include <string>
#include <map>
#include <exception>
#include <vector>
#include "User.hpp"
#include "Server.hpp"

class User;
class Channel
{
	private:
		std::string							_key;
		std::string							_channelName;
		std::string							_topic;
		bool								_isInviteOnly;
		unsigned int						_maximumUsers;
	public:
		std::map<char, bool>				_mode;
		std::map<int, User *>				_liveUsers;
		std::vector<User *>					_operators;
		std::vector<User *>					_invitedUsers;

		Channel (std::string name);
		Channel (std::string name, std::string key);
		~Channel();
		std::string 		getName() const;
		std::string 		getTopic() const;
		std::string 		getKey() const;
		std::map<char, bool>getMode() const;
		unsigned int		getMaximumUsers() const;
		void				setName(std::string name);
		void				setTopic(std::string topic);
		void				setKey(std::string key);
		void				setMaximumUsers(unsigned int maximumUsers);
		void				setMode(std::map<char, bool> mode);
};
