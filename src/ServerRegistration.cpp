#include "Server.hpp"

#include <iostream>

// Validates the client's password and updates its authentication status.
// Sends an IRC error reply if the password is missing or incorrect.
void Server::handlePass(Client *client, const std::string& message)
{
	std::string args;

	args = trimSpaces(getCommandArgs(message));
	if (client->isRegistered())
	{
		sendNumeric(client, "462", ":You may not reregister");
		return ;
	}
	if (args.empty())
	{
		sendNumeric(client, "461", "PASS :Not enough parameters");
		return ;
	}
	if (args != _password)
	{
		sendNumeric(client, "464", ":Password incorrect");
		return ;
	}
	client->setPassOk(true);
	checkRegistration(client);
}

// Checks nickname availability, assigns the requested nickname,
// and attempts to complete the client's registration.
void Server::handleNick(Client *client, const std::string& message)
{
	std::string nickname;

	//nickname = getCommandArgs(message);
	std::string args = getCommandArgs(message);
	size_t start = args.find_first_not_of(" \t");
	if (start != std::string::npos)
	{
		size_t end = args.find_first_of(" \t", start);
		nickname = args.substr(start, end - start);
	}
	if (nickname.empty())
	{
		sendNumeric(client, "431", ":No nickname given");
		return ;
	}
	if (isNicknameTaken(nickname))
	{
		sendNumeric(client, "433", nickname + " :Nickname is already in use");
		return ;
	}
	client->setNickname(nickname);
	checkRegistration(client);
}

// Parses and validates the USER command parameters,
// then stores the username and real name for registration.
void	Server::handleUser(Client *client, const std::string& message)
{
	std::string	args;
	std::string	username;
	std::string	realname;
	size_t		firstSpace;
	size_t		secondSpace;
	size_t		thirdSpace;
	size_t		colonPos;

	if (client->isRegistered())
	{
		sendNumeric(client, "462", ":You may not reregister");
		return ;
	}
	args = trimSpaces(getCommandArgs(message));
	if (args.empty())
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	firstSpace = args.find(' ');
	if (firstSpace == std::string::npos)
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	secondSpace = args.find(' ', firstSpace + 1);
	if (secondSpace == std::string::npos)
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	thirdSpace = args.find(' ', secondSpace + 1);
	if (thirdSpace == std::string::npos)
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	username = args.substr(0, firstSpace);
	colonPos = args.find(':', thirdSpace + 1);
	if (colonPos == std::string::npos)
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	realname = args.substr(colonPos + 1);
	if (username.empty() || realname.empty())
	{
		sendNumeric(client, "461", "USER :Not enough parameters");
		return ;
	}
	client->setUsername(username);
	client->setRealname(realname);
	checkRegistration(client);
}

// Completes client registration when the password, nickname,
// and username are valid, then sends the IRC welcome reply.
void Server::checkRegistration(Client *client)
{
	if (client->isRegistered())
		return ;
	if (client->isPassOk() == true
		&& client->getNickname().empty() == false
		&& client->getUsername().empty() == false)
	{
		client->setRegistered(true);
		sendNumeric(client, "001", ":Welcome to the ft_irc server, "
			+ client->getNickname());
	}
}

// Checks whether a nickname is already assigned to a connected client.
bool Server::isNicknameTaken(const std::string& nickname) const
{
	std::map<int, Client*>::const_iterator it;

	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return (true);
	}
	return (false);
}

// Finds a connected client by nickname and returns its pointer,
// or NULL if no matching client exists.
Client *Server::findClientByNickname(const std::string& nickname) const
{
	std::map<int, Client*>::const_iterator it;

	for (it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == nickname)
			return (it->second);
	}
	return (NULL);
}
