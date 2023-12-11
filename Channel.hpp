#pragma once

#include <stdio.h>
#include <string>
#include <map>

class Channel
{
	private:
		std::string				_name;
		std::map<char, bool>	_mode;
		unsigned int			_maximumUsers;
		std::string				_topic;
		std::string				_key;
public:
	Channel (std::string name);
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
