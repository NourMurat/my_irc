#include "Channel.hpp"
#include "User.hpp"
#include <cstdlib> // Для rand() и srand()
#include <ctime>   // Для time()


/* Constructor and Destructor */

Channel::Channel(std::string nameValue, User *ownerValue) : 
        name(nameValue), pass(""), topic(""), 
        owner(ownerValue), limit(0), inviteOnly(false) {}

Channel::~Channel() {}


/* Getters */

std::string     Channel::getName() const { return name; }
std::string     Channel::getPass() const { return pass; }
std::string     Channel::getTopic() const { return topic; }
User*           Channel::getOwner() const { return owner; }
size_t          Channel::getLimit() const { return limit; }
size_t          Channel::getSize()const { return members.size(); }
bool            Channel::getTopicRestrictions() const { return topicRestrictions; }

const std::map<std::string, User* >&    Channel::getMembers() const { return members; }
const std::map<std::string, User* >&    Channel::getInvited() const { return invited; }
const std::map<std::string, User* >&    Channel::getOperators() const { return operators; }
const std::map<std::string, User* >&    Channel::getBanned() const { return banned; }


/* Setters */

void        Channel::setName(std::string nameValue) { name = nameValue; }
void        Channel::setPass(std::string passValue) { pass = passValue; }
void        Channel::setTopic(std::string topicValue) { pass = topicValue; }
void        Channel::setLimit(size_t limitValue) { limit = limitValue; }
void        Channel::setInviteOnly(bool value) { inviteOnly = value; }
void        Channel::setTopicRestrictions(bool value) { topicRestrictions = value; }
/* Channel Methods */

// Whether the channel is an invitation-only channel
bool        Channel::isInviteOnly() const { return inviteOnly; }

// Compare the provided password with the channel password
bool        Channel::checkPassword(const std::string& password) const { return pass == password; }

// Whether the User is in the list of channel members, operators, invited channel members
bool        Channel::isMember(User* client) const { return members.find(client->getNickname()) != members.end(); }
bool        Channel::isOperator(User* client) const { return operators.find(client->getNickname()) != operators.end(); }
bool        Channel::isInvited(User* client) const { return invited.find(client->getNickname()) != invited.end(); }
bool        Channel::isOwner(User* client) const { return owner == client; }

// Whether the channel is empty
bool        Channel::isEmpty() const { return members.empty() && operators.empty(); }

// whether there is a limit on the number of User in the channel
bool        Channel::hasUserLimit() const { return limit > 0; }

// whether the channel has a set key (password)
bool        Channel::hasChannelKey() const { return !pass.empty(); }

// bool        Channel::hasTopicRestrictions() const { return topic; }

// Adding a User to the member map if it is not already there
void        Channel::addMember(User* client) 
{
    if (inviteOnly)
    { // If the channel is invitation-only
        if (invited.find(client->getNickname()) == invited.end())
        {
            // The User is not invited, send a message
            std::string message = "NOTICE " + name + " :" + client->getNickname() + " :You're not invited to join this channel";
            client->write(message);
            return;
        }
    }

    if (members.find(client->getNickname()) == members.end()) 
    {
        // if (members.find(client->getNickname()) == owner)
        //     return;
        members[client->getNickname()] = client;
        std::cout << MAGENTA << "DEBUGG:: ADD MEMBER (" << members[client->getNickname()]->getNickname() << ") IN THE NEW CHANNEL (" << getName() << ") !!!" << RESET << "\n";
    }
}

// Adding a User to the invite map if he is not already there
void        Channel::addInvited(User* client) 
{
    if (invited.find(client->getNickname()) == invited.end()) 
    {
        invited[client->getNickname()] = client;
    }
}

// Adding a client to the operator map if it is not already there
void        Channel::addOperator(User* client, User* invoker) 
{
    // Access rights check: owner or operators can add operators
    if (operators.find(invoker->getNickname()) != operators.end() || owner == invoker) 
    {
        if (operators.find(client->getNickname()) == operators.end()) 
        {
            operators[client->getNickname()] = client;
            members.erase(client->getNickname());
            std::string message = "NOTICE " + name + " :(" + client->getNickname() + ") has become Operator in this chanel!";
            broadcast(message);
        }
    }
}

// Adding a User to the banned map if it is not already there
void        Channel::addBanned(User* client, User* invoker, const std::string& reason) 
{
    // Checking access rights: owner or operators can ban customers
    if (operators.find(invoker->getNickname()) != operators.end() || owner == invoker) 
    {
        if (banned.find(client->getNickname()) == banned.end()) 
        {
            banned[client->getNickname()] = client;

            // Sending ban notification to IRC chat according to RFC 2812 protocol
            std::string notice = "NOTICE " + name + " :" + client->getNickname() + " has been banned by " + invoker->getNickname() + " (" + reason + ")";
            // Отправляем 'notice' всем клиентам в этом канале
            broadcast(notice);

            // // Saving information about the ban in the logbook
            // std::string logMessage = client->getNickname() + " был забанен оператором " + invoker->getNickname() + " по причине: " + reason;
            // log(logMessage);
        }
    }
}

// Removing a member from map members
int        Channel::removeUserFromChannel(User* client) 
{
    client_iterator itMember = members.find(client->getNickname());
    std::cout << MAGENTA << "DEBUGG:: check member (" << itMember->first << ") !!!" << RESET << "\n";
    
    operator_iterator itOperator = operators.find(client->getNickname());
    std::cout << MAGENTA << "DEBUGG:: check operator (" << itOperator->first << ") !!!" << RESET << "\n";
    
    if (itMember != members.end()) 
    {
        std::cout << MAGENTA << "DEBUGG:: REMOVE MEMBER (" << members[client->getNickname()]->getNickname() << ") FROM THE CHANNEL (" << getName() << ") !!!" << RESET << "\n";
        std::string channelMsg = ":" + client->getNickname() + " PART " + name + " :leaving" + "\r\n";
		broadcast(channelMsg, client);
        members.erase(itMember);

        // std::string logMessage = client->getNickname() + " has left channel " + name;
        // log(logMessage);
    }
    else if (itOperator != operators.end())
    {
        std::cout << MAGENTA << "DEBUGG:: REMOVE OPERATOR (" << operators[client->getNickname()]->getNickname() << ") FROM THE CHANNEL (" << getName() << ") !!!" << RESET << "\n";
        std::string channelMsg = ":" + client->getNickname() + " PART " + name + " :leaving" + "\r\n";
		broadcast(channelMsg, client);
        takeOperatorPrivilege(itOperator->second);
        members.erase(itOperator);

        // std::string logMessage = client->getNickname() + " has left channel " + name;
        // log(logMessage);
    }
    else if (owner == client)
    {
        std::cout << MAGENTA << "DEBUGG:: REMOVE OWNER (" << owner->getNickname() << ") FROM THE CHANNEL (" << name << ") !!!" << RESET << "\n";
        std::string channelMsg = ":" + client->getNickname() + " PART " + name + " :leaving" + "\r\n";
        broadcast(channelMsg, client);

        // Если в канале есть операторы
        if (!operators.empty())
        {
            int randomIndex = rand() % operators.size(); // Получаем случайный индекс
            operator_iterator itOperator = operators.begin();
            std::advance(itOperator, randomIndex); // Перемещаем итератор на случайную позицию
            
            owner = itOperator->second; // Назначаем нового хозяина
            takeOperatorPrivilege(itOperator->second);
            std::cout << MAGENTA << "DEBUGG:: (" << owner->getNickname() << ") IS A NEW OWNER OF THE CHANNEL (" << name << ") !!!" << RESET << "\n";
            std::string msg = ":(" + client->getNickname() + ") no more owner of the channel (" + name + ") New owner is (" + owner->getNickname() + ")\r\n";
            std::string channelMsg = msg + "NOTICE " + name + " :(" + client->getNickname() + ") no more owner of the channel (" + name + ") New owner is (" + owner->getNickname() + ")";
            broadcast(channelMsg, client);
        }
        // Если операторов нет, но есть обычные члены
        else if (!members.empty())
        {
            int randomIndex = rand() % members.size(); // Получаем случайный индекс
            client_iterator itMember = members.begin();
            std::advance(itMember, randomIndex); // Перемещаем итератор на случайную позицию
            
            owner = itMember->second; // Назначаем нового хозяина
            std::cout << MAGENTA << "DEBUGG:: (" << owner->getNickname() << ") IS A NEW OWNER OF THE CHANNEL (" << name << ") !!!" << RESET << "\n";
            std::string msg = ":(" + client->getNickname() + ") no more owner of the channel (" + name + ") New owner is (" + owner->getNickname() + ")\r\n";
            std::string channelMsg = msg + "NOTICE " + name + " :(" + client->getNickname() + ") no more owner of the channel (" + name + ") New owner is (" + owner->getNickname() + ")";
            broadcast(channelMsg, client);
        }
        // Если хозяин - единственный член канала
        else
        {
            // Логика удаления канала
            std::cout << MAGENTA << "DEBUGG:: the channel (" + name + ") is closing... " << RESET << "\n";
            return 1;
        }
    // Deleting a channel a User is a member of
    client->removeChannelOfClient(name);
    }
    return 0;
}

// Removing an operator from map operators
void        Channel::takeOperatorPrivilege(User* target) 
{
    // // Check if the User is the channel owner
    // if (owner == target) {
    //     target->write("NOTICE " + name + " :You are not authorized to remove owner.");
    //     return;
    // }
    
    operator_iterator it = operators.find(target->getNickname());
    if (it != operators.end()) 
    {
        
        // Adding an operator to the ordinary membership map
        members[target->getNickname()] = target;
        
        operators.erase(it);

        std::string message = "NOTICE " + name + " :(" + target->getNickname() + ") no more Operator in this chanel!";
        broadcast(message);
        
        // std::string logMessage = target->getNickname() + " is no longer an operator in channel " + name;
        // log(logMessage);
    }
}

// Removing an invitee from map invited
void        Channel::removeInvited(User* client) 
{
    client_iterator it = invited.find(client->getNickname());
    if (it != invited.end()) 
    {
        std::string message = "INVITE " + client->getNickname() + " :" + name;
        client->write(message);
        
        invited.erase(it);

        // std::string logMessage = client->getNickname() + " is no longer invited to channel " + name;
        // log(logMessage);
    }
}

// Removing a banned person from map banned (unbanning)
void        Channel::removeBanned(User* client) 
{
    client_iterator it = banned.find(client->getNickname());
    if (it != banned.end()) 
    {
        std::string message = "MODE " + name + " +b " + client->getNickname();
        broadcast(message);
        
        banned.erase(it);

        // std::string logMessage = client->getNickname() + " has been unbanned from channel " + name;
        // log(logMessage);
    }
}

void        Channel::removeUserLimit() { limit = 0; }
void        Channel::removeChannelKey() { pass.clear(); }
void        Channel::removeTopicRestrictions() { topic.clear(); }



void        Channel::broadcast(const std::string& message)
{
    // Send a message to all channel members
    client_iterator it = members.begin();
    while (it != members.end()) 
    {
        // std::string prefixNickname = "+" + senderNickname;
        it->second->write(message);
        it++;
    }

    // Sending a message to the channel operators
    it = operators.begin();
    while (it != operators.end()) 
    {
        // std::string prefixNickname = "@" + senderNickname;
        it->second->write(message);
        it++;
    }

    // Send a message to the owner of the channel
    if (owner != NULL) 
    {
        // std::string prefixNickname = "@" + senderNickname;
        owner->write(message);
    }
}

void        Channel::broadcast(const std::string& message, User* exclude)
{
    // Send the message to all members of the channel, excluding the specified User
    client_iterator it = members.begin();
    while (it != members.end()) 
    {
        if (it->second != exclude) 
        {
            // std::string prefixNickname = ":+" + senderNickname;
            it->second->write(message);
        }
        it++;
    }

    // Send a message to the channel operators, excluding the specified User
    it = operators.begin();
    while (it != operators.end()) 
    {
        if (it->second != exclude) 
        {
            // std::string prefixNickname = "@" + senderNickname;
            it->second->write(message);
        }
        it++;
    }

    // Send a message to the channel owner, excluding the specified User
    if (owner != NULL && owner != exclude) 
    {
        // std::string prefixNickname = "@" + senderNickname;
        owner->write(message);
    }
}

void        Channel::kick(User* client, User* target, const std::string& reason) 
{
    // Check if the client has kick privileges (owner or operator)
    if (operators.find(client->getNickname()) != operators.end() || owner == client) 
    {
        // Check if the target is the channel owner
        if (target == owner) 
        {
            client->write("NOTICE " + name + " :You cannot kick the channel owner.");
            return;
        }

        // Search for the target in different maps
        client_iterator itMember = members.find(target->getNickname());
        client_iterator itOperator = operators.find(target->getNickname());
        client_iterator itInvited = invited.find(target->getNickname());
        client_iterator itBanned = banned.find(target->getNickname());

        // Perform kick if the target is found in any of the maps
        if (itMember != members.end() || itOperator != operators.end() ||
            itInvited != invited.end() || itBanned != banned.end()) 
        {

            // Send kick notification to IRC chat
            std::string kickMessage = "KICK " + name + " " + target->getNickname() + " :" + reason;
            broadcast(kickMessage);

            // Remove the target from appropriate maps using existing functions
            if (itMember != members.end()) removeUserFromChannel(target);
            if (itOperator != operators.end()) takeOperatorPrivilege(target);
            if (itInvited != invited.end()) removeInvited(target);
            if (itBanned != banned.end()) removeBanned(target);

            // // Write a log entry for the kick
            // std::string logMessage = target->getNickname() + " was kicked from channel " + name + " by " + client->getNickname() + " (" + reason + ")";
            // log(logMessage);
        }
    }
}
