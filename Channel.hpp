#pragma once

#include "Irc.hpp"
#include <map>
#include <string>

class User;
class Server;

class Channel 
{
	// A client_iterator synonym is created to make the code easier to read	
	typedef std::map<std::string, User*>::iterator client_iterator;
	typedef std::map<std::string, User*>::iterator operator_iterator;

private:
	std::string		name; //name of the channel
	std::string 	pass; //password to join the channel
	std::string		topic; // topic of the channel
	User*			owner; //A pointer to the User who is the owner of the channel
	size_t 			limit; //The limit on the number of members allowed in the channel
	bool 			inviteOnly; // A Boolean flag indicating whether the channel is by invitation or not
	bool			topicRestrictions; // A Boolean flag indicating whether the topic can be changed by anyone or not

public:
	Channel( std::string, User*);
	~Channel();

	/* maps as private class members will be initialized
	with null by the Constructor when the Class is created */
	std::map<std::string, User *> members;
	std::map<std::string, User *> invited;
	std::map<std::string, User *> operators;
	std::map<std::string, User *> banned;
	
/* Getters */

    std::string		getName() const;
    std::string		getPass() const;
	std::string		getTopic() const;
	User*			getOwner() const;
    size_t			getLimit() const;
    size_t			getSize() const;
	// Get a maps of all channel members, as well as a list of invited Users, 
	// all operators and banned cUsers
    const std::map<std::string, User *>&	getMembers() const;
	const std::map<std::string, User *>&	getInvited() const;
    const std::map<std::string, User *>&	getOperators() const;
	const std::map<std::string, User *>&	getBanned() const;
	size_t      							countUsers(Channel* channel) const;


/* Setters */
//if you want to change name, password, topic or limits
	void		setName(std::string nameValue);
	void		setPass(std::string passValue);
	void		setTopic(std::string topicValue);
	void		setLimit(size_t limitValue);
	void		setInviteOnly(bool value);
	void		setTopicRestrictions(bool value);

/* Channel Methods */

	bool		isInviteOnly() const;
	bool 		checkPassword(const std::string& password) const;
	bool        isMember(User* client) const;
	bool        isOperator(User* client) const;
	bool		isInvited(User* client) const;
	bool		isOwner(User* client) const;
	bool 		isEmpty() const;
	bool		hasUserLimit() const;
	bool		hasChannelKey() const;
	bool		hasTopicRestrictions() const;

	// Add the corresponding Users to the maps we need
	int 		addMember(User* client);
	void 		addInvited(User* client);
	void 		addOperator(User* client, User* invoker);
	void 		addBanned(User* client, User* invoker, const std::string& reason);

	// Remove the corresponding Users from the map we need
	int 		removeUserFromChannel(User* client);
	void 		takeOperatorPrivilege(User* client);
	void 		removeInvited(User* client);
	void 		removeBanned(User* client);
	void		removeUserLimit();
	void		removeChannelKey();
	void		removeOwnerFromChannel();

	//sends a message to all Users
    void		broadcast(const std::string& message);
	//sedns a message to all Users except a specific User(s)
    void		broadcast(const std::string& message, User* exclude);
		
};
