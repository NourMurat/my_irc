#include "Channel.hpp"
#include "User.hpp"


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
void        Channel::removeMember(User* client) 
{
    client_iterator it = members.find(client->getNickname());
    if (it != members.end()) 
    {
        std::string message = "PART " + name + " :" + client->getNickname();
        broadcast(message);
        members.erase(it);

        // std::string logMessage = client->getNickname() + " has left channel " + name;
        // log(logMessage);
    }
    // Deleting a channel a User is a member of
    client->removeChannelOfClient(name);
}

// Removing an operator from map operators
void        Channel::removeOperator(User* client) 
{
    // Check if the User is the channel owner
    if (owner != client) {
        client->write("NOTICE " + name + " :You are not authorized to remove operators.");
        return;
    }
    
    client_iterator it = operators.find(client->getNickname());
    if (it != operators.end()) 
    {
        std::string message = "MODE " + name + " -o " + client->getNickname();
        broadcast(message);
        
        // Adding an operator to the ordinary membership map
        members[client->getNickname()] = client;
        
        operators.erase(it);
        
        // std::string logMessage = client->getNickname() + " is no longer an operator in channel " + name;
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
        it->second->write(message);
        it++;
    }

    // Sending a message to the channel operators
    it = operators.begin();
    while (it != operators.end()) 
    {
        it->second->write(message);
        it++;
    }

    // Send a message to the owner of the channel
    if (owner != NULL) 
    {
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
            it->second->write(message);
        }
        it++;
    }

    // Send a message to the channel owner, excluding the specified User
    if (owner != NULL && owner != exclude) 
    {
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
            if (itMember != members.end()) removeMember(target);
            if (itOperator != operators.end()) removeOperator(target);
            if (itInvited != invited.end()) removeInvited(target);
            if (itBanned != banned.end()) removeBanned(target);

            // // Write a log entry for the kick
            // std::string logMessage = target->getNickname() + " was kicked from channel " + name + " by " + client->getNickname() + " (" + reason + ")";
            // log(logMessage);
        }
    }
}
