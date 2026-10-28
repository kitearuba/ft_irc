#include "Server.hpp"

// Queues an IRC message with CRLF termination
// and enables POLLOUT so it can be sent to the client.
void Server::sendReply(Client *client, const std::string& message)
{
	std::string response;

	response = message + "\r\n";
	client->appendOutput(response);
	enablePollOut(client->getFd());
}

// Formats an IRC numeric reply with the server name,
// reply code, and target nickname before sending it.
void Server::sendNumeric(Client *client, const std::string& code,
	const std::string& message)
{
	std::string target;
	std::string response;

	if (client->getNickname().empty())
		target = "*";
	else
		target = client->getNickname();
	response = ":ircserv " + code + " " + target + " " + message;
	sendReply(client, response);
}

// Sends numeric error 421 for an unrecognized IRC command.
void Server::sendUnknownCommand(Client *client, const std::string& command)
{
	sendNumeric(client, "421", command + " :Unknown command");
}

// Sends numeric error 451 when a command requires registration.
void Server::sendNotRegistered(Client *client)
{
	sendNumeric(client, "451", ":You have not registered");
}
