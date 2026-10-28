#include "Channel.hpp"

// Initializes an empty channel with all modes disabled.
Channel::Channel() :
	_name(""),
	_topic(""),
	_password(""),
	_inviteOnly(false),
	_topicRestricted(false),
	_userLimit(0)
{
}

// Creates a named channel with default settings and no active modes.
Channel::Channel(const std::string& name) :
	_name(name),
	_topic(""),
	_password(""),
	_inviteOnly(false),
	_topicRestricted(false),
	_userLimit(0)
{
}

// Creates a channel by copying another channel's state.
Channel::Channel(const Channel& other)
{
	*this = other;
}

// Copies all channel properties, members, operators, and invitations
// while protecting against self-assignment.
Channel& Channel::operator=(const Channel& other)
{
	if (this != &other)
	{
		_name = other._name;
		_topic = other._topic;
		_password = other._password;
		_members = other._members;
		_operators = other._operators;
		_inviteOnly = other._inviteOnly;
		_topicRestricted = other._topicRestricted;
		_userLimit = other._userLimit;
        _invited = other._invited;
	}
	return (*this);
}

// Destroys the channel and releases its managed containers.
Channel::~Channel()
{
}

// Returns the channel's name.
std::string Channel::getName() const
{
	return (_name);
}

// Returns the channel's current topic.
std::string Channel::getTopic() const
{
	return (_topic);
}

// Returns the number of clients currently in the channel.
size_t Channel::getMemberCount() const
{
	return (_members.size());
}

// Updates the channel's topic.
void Channel::setTopic(const std::string& topic)
{
	_topic = topic;
}

// Checks whether the channel has a non-empty topic.
bool Channel::hasTopic() const
{
	return (_topic.empty() == false);
}

// Returns a read-only reference to the channel's member list.
const std::vector<Client*>& Channel::getMembers() const
{
	return (_members);
}

// Adds a client to the channel if not already a member.
void Channel::addMember(Client *client)
{
	if (isMember(client) == false)
		_members.push_back(client);
}

// Removes a client from the channel, including operator
// privileges and any pending invitation.
void Channel::removeMember(Client *client)
{
	std::vector<Client*>::iterator it;

	for (it = _members.begin(); it != _members.end(); ++it)
	{
		if (*it == client)
		{
			_members.erase(it);
			break ;
		}
	}
	removeOperator(client);
	removeInvited(client);
}

// Checks whether a client belongs to the channel.
bool Channel::isMember(Client *client) const
{
	std::vector<Client*>::const_iterator it;

	for (it = _members.begin(); it != _members.end(); ++it)
	{
		if (*it == client)
			return (true);
	}
	return (false);
}

// Grants channel operator privileges to a client if not already granted.
void Channel::addOperator(Client *client)
{
	if (isOperator(client) == false)
		_operators.push_back(client);
}


// Removes channel operator privileges from a client.
void Channel::removeOperator(Client *client)
{
	std::vector<Client*>::iterator it;

	for (it = _operators.begin(); it != _operators.end(); ++it)
	{
		if (*it == client)
		{
			_operators.erase(it);
			break ;
		}
	}
}

// Checks whether a client has channel operator privileges.
bool Channel::isOperator(Client *client) const
{
	std::vector<Client*>::const_iterator it;

	for (it = _operators.begin(); it != _operators.end(); ++it)
	{
		if (*it == client)
			return (true);
	}
	return (false);
}

// Builds a space-separated list of channel nicknames,
// prefixing operators with '@' for IRC NAMES replies.
std::string Channel::getNamesList() const
{
	std::vector<Client*>::const_iterator	it;
	std::string							names;

	for (it = _members.begin(); it != _members.end(); ++it)
	{
		if (names.empty() == false)
			names += " ";
		if (isOperator(*it))
			names += "@";
		names += (*it)->getNickname();
	}
	return (names);
}

// Placeholder for channel broadcasting; currently performs no action.
void Channel::broadcast(const std::string& message, Client *except)
{
	(void)message;
	(void)except;
}

// Adds a client to the channel's invitation list if not already invited.
void Channel::addInvited(Client *client)
{
	if (isInvited(client) == false)
		_invited.push_back(client);
}

// Removes a client's pending channel invitation.
void Channel::removeInvited(Client *client)
{
	for (std::vector<Client*>::iterator it = _invited.begin();
		it != _invited.end(); ++it)
	{
		if (*it == client)
		{
			_invited.erase(it);
			return ;
		}
	}
}

// Checks whether a client has a pending channel invitation.
bool Channel::isInvited(Client *client) const
{
	for (std::vector<Client*>::const_iterator it = _invited.begin();
		it != _invited.end(); ++it)
	{
		if (*it == client)
			return (true);
	}
	return (false);
}

// Enables or disables invite-only channel mode (+i).
void	Channel::setInviteOnly(bool value)
{
	_inviteOnly = value;
}

// Returns whether invite-only channel mode (+i) is enabled.
bool	Channel::isInviteOnly() const
{
	return (_inviteOnly);
}

// Enables or disables topic restriction mode (+t).
void	Channel::setTopicRestricted(bool value)
{
	_topicRestricted = value;
}

// Returns whether topic restriction mode (+t) is enabled.
bool	Channel::isTopicRestricted() const
{
	return (_topicRestricted);
}

// Sets the channel password used by mode +k.
void	Channel::setPassword(const std::string& password)
{
	_password = password;
}

// Returns the channel's current password.
std::string	Channel::getPassword() const
{
	return (_password);
}

// Checks whether the channel has a password (+k).
bool	Channel::hasPassword() const
{
	return (_password.empty() == false);
}

// Removes the channel password and disables mode +k.
void	Channel::clearPassword()
{
	_password = "";
}

// Sets the maximum number of channel members for mode +l.
void	Channel::setUserLimit(int limit)
{
	_userLimit = limit;
}

// Returns the configured channel user limit.
int	Channel::getUserLimit() const
{
	return (_userLimit);
}

// Checks whether a positive user limit (+l) is active.
bool	Channel::hasUserLimit() const
{
	return (_userLimit > 0);
}

// Removes the channel user limit and disables mode +l.
void	Channel::clearUserLimit()
{
	_userLimit = 0;
}