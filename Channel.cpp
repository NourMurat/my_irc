#include "Channel.hpp"
#include "User.hpp"
#include <cstdlib> // Для rand() и srand()
#include <ctime>   // Для time()


/* Constructor and Destructor */

Channel::Channel(std::string nameValue, User *ownerValue) : 
        name(nameValue), pass(""), topic(""), 
        owner(ownerValue), limit(0), inviteOnly(false), topicRestrictions(false) {}

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
void        Channel::setTopic(std::string topicValue) { topic = topicValue; }
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

bool        Channel::hasTopicRestrictions() const { return topicRestrictions; }

// Adding a User to the member map if it is not already there
int        Channel::addMember(User* client) 
{
    if (inviteOnly)
    { // If the channel is invitation-only
        if (invited.find(client->getNickname()) == invited.end())
        {
            // The User is not invited, send a message
            std::string message = "ERROR :You're NOT INVITED to join this channel\r\n";
            client->write(message);
            return 1;
        }
    }

    if (members.find(client->getNickname()) == members.end()) 
    {
        members[client->getNickname()] = client;
        std::cout << GREEN << "LOG:: ADD MEMBER (" << members[client->getNickname()]->getNickname() 
                << ") IN THE NEW CHANNEL (" << getName() << ") !!!" << RESET << "\n";
        if (inviteOnly && invited.find(client->getNickname()) != invited.end())
        {
            invited.erase(client->getNickname());
        }
    }
    return 0;
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
    if (operators.find(invoker->getNickname()) != operators.end() || owner == invoker) 
    {
        // Проверяем, является ли пользователь уже оператором
        if (operators.find(client->getNickname()) != operators.end())
        {
            // Пользователь уже является оператором, отправляем сообщение об ошибке
            std::string message = "ERROR :User (" + client->getNickname() + ") is already an Operator in this channel.\r\n";
            invoker->write(message);
            return;
        }

        // Проверяем, есть ли пользователь в списке членов
        if (members.find(client->getNickname()) != members.end())
        {
            operators[client->getNickname()] = client;
            std::string nick = client->getNickname();
            members.erase(nick);

            if (members.find(nick) == members.end())
            {
                // Успешно удален из списка членов и добавлен в операторы
                std::string broadcastMessage = "NOTICE " + name + " :(" + nick + ") has become Operator in this channel!\r\n";
                broadcast(broadcastMessage);
            }
            else
            {
                // Ошибка: пользователь не был удален из списка членов
                std::string errorMessage = "ERROR :Failed to add (" + nick + ") as an Operator.\r\n";
                broadcast(errorMessage);
            }
        }
        else
        {
            // Пользователь не найден в списке членов
            std::string errorMessage = "ERROR :(" + client->getNickname() + ") is not a member of this channel, cannot be added as Operator.\r\n";
            invoker->write(errorMessage);
        }
    }
    else
    {
        // Отправитель не имеет права назначать операторов
        std::string errorMessage = "ERROR :You do not have the permission to add Operators in this channel.\r\n";
        invoker->write(errorMessage);
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
        std::cout << GREEN << "LOG:: REMOVE MEMBER (" << members[client->getNickname()]->getNickname() << ") FROM THE CHANNEL (" << getName() << ") !!!" << RESET << "\n";
        std::string channelMsg = ":" + client->getNickname() + " PART " + name + " :Goodbye member!" + "\r\n";
		broadcast(channelMsg, client);
        members.erase(itMember);
    }
    else if (itOperator != operators.end())
    {
        std::cout << GREEN << "LOG:: REMOVE OPERATOR (" << operators[client->getNickname()]->getNickname() << ") FROM THE CHANNEL (" << getName() << ") !!!" << RESET << "\n";
        std::string channelMsg = ":" + client->getNickname() + " PART " + name + " :Goodbye operator!" + "\r\n";
		broadcast(channelMsg, client);
        operators.erase(itOperator);
    }
    else if (owner == client)
    {
        std::cout << GREEN << "LOG:: REMOVE OWNER (" << owner->getNickname() << ") FROM THE CHANNEL (" << name << ") !!!" << RESET << "\n";
        std::string channelMsg = ":" + client->getNickname() + " PART " + name + " :Goodbye owner!" + "\r\n";
        broadcast(channelMsg);

        // Если в канале есть операторы
        if (!operators.empty())
        {
            int randomIndex = rand() % operators.size(); // Получаем случайный индекс
            operator_iterator itOperator = operators.begin();
            std::advance(itOperator, randomIndex); // Перемещаем итератор на случайную позицию
            
            owner = itOperator->second; // Назначаем нового хозяина
            std::string nickOperator = itOperator->first;
            if (operators.find(nickOperator) == operators.end())
            {
                std::cout << GREEN << "LOG:: (" << owner->getNickname() << ") IS A NEW OWNER OF THE CHANNEL (" << name << ") !!!" << RESET << "\n";
                std::string channelMsg = "NOTICE " + name + " :" + client->getNickname() + " no more owner of the channel " + name + ". New owner is " + owner->getNickname() + "\r\n";
                broadcast(channelMsg, client);
            }
            operators.erase(itOperator);
        }
        // Если операторов нет, но есть обычные члены
        else if (!members.empty())
        {
            int randomIndex = rand() % members.size(); // Получаем случайный индекс
            client_iterator itMember = members.begin();
            std::advance(itMember, randomIndex); // Перемещаем итератор на случайную позицию
            
            owner = itMember->second; // Назначаем нового хозяина
            std::string nickMember = itMember->first;
            if (members.find(nickMember) == members.end())
            {
                std::cout << GREEN << "LOG:: (" << owner->getNickname() << ") IS A NEW OWNER OF THE CHANNEL (" << name << ") !!!" << RESET << "\n";
                std::string channelMsg = "NOTICE " + name + " :" + client->getNickname() + " no more owner of the channel " + name + ". New owner is " + owner->getNickname() + "\r\n";
                broadcast(channelMsg, client);
            }
            members.erase(itMember);
        }
        // Если хозяин - единственный член канала
        else
        {
            // Логика удаления канала
            std::cout << GREEN << "LOG:: the channel (" + name + ") was closed!" << RESET << "\n";
            return 1;
        }
    }
    return 0;
}

size_t      Channel::countUsers(Channel* channel) const
{
    return (channel->members.size() + channel->operators.size() + 1);
}

// Removing an operator from map operators
void        Channel::takeOperatorPrivilege(User* target) 
{
    operator_iterator it = operators.find(target->getNickname());
    if (it != operators.end()) 
    {
        // Adding an operator to the ordinary membership map
        members[target->getNickname()] = target;
        std::string message = "NOTICE " + name + " :(" + target->getNickname() + ") no more Operator in this chanel!\r\n";
        broadcast(message);
        operators.erase(it);
    }
}

// Removing an invitee from map invited
void        Channel::removeInvited(User* client) 
{
    client_iterator it = invited.find(client->getNickname());
    if (it != invited.end()) 
    {
        std::string message = "INVITE " + client->getNickname() + " :" + name + "\r\n";
        client->write(message);
        invited.erase(it);
    }
}

// Removing a banned person from map banned (unbanning)
void        Channel::removeBanned(User* client) 
{
    client_iterator it = banned.find(client->getNickname());
    if (it != banned.end()) 
    {
        std::string message = "MODE " + name + " +b " + client->getNickname() + "\r\n";
        broadcast(message);
        banned.erase(it);
    }
}

void        Channel::removeUserLimit() { limit = 0; }
void        Channel::removeChannelKey() { pass.clear(); }

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
