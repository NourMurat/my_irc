#include "Channel.hpp"

Channel::Channel(std::string name)
{
    this->_name = name;
    this->_maximumUsers = 512;
    this->_mode['i'] = false;
    this->_mode['t'] = false;
    this->_mode['k'] = false;
    this->_mode['o'] = false;
    this->_mode['l'] = false;
}

Channel::~Channel()
{
}

// GETTERS AND SETTERS
std::string Channel::getName() const
{
    return this->_name;
}

std::string Channel::getTopic() const
{
    return this->_topic;
}

std::string Channel::getKey() const
{
    return this->_key;
}

unsigned int Channel::getMaximumUsers() const
{
    return this->_maximumUsers;
}

std::map<char, bool> Channel::getMode() const
{
    return this->_mode;
}

void Channel::setName(std::string name)
{
    this->_name = name;
}

void Channel::setTopic(std::string topic)
{
    this->_topic = topic;
}

void Channel::setKey(std::string key)
{
    this->_key = key;
}

void Channel::setMaximumUsers(unsigned int maximumUsers)
{
    this->_maximumUsers = maximumUsers;
}

void Channel::setMode(std::map<char, bool> mode)
{
    this->_mode = mode;
}
