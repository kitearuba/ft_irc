#include "Client.hpp"

// Initializes an empty client with a default invalid socket
// and clears all registration and communication data.
Client::Client()
{
  _fd = -1;
  _nickname = "";
  _username = "";
  _realname = "";
  _buffer = "";
	_outBuffer = "";
  _passOk = false;
  _registered = false;
}

// Initializes a client with the specified socket descriptor
// and sets its registration and communication state to default values.
Client::Client(int fd)
{
  _fd = fd;
  _nickname = "";
  _username = "";
  _realname = "";
  _buffer = "";
	_outBuffer = "";
  _passOk = false;
  _registered = false;
}

// Creates a client by copying the state of another client.
Client::Client(const Client& other)
{
  *this = other;
}

// Copies the socket descriptor, identity, buffers, and registration state
// from another client while protecting against self-assignment.
Client& Client::operator=(const Client& other)
{
  if (this != &other)
  {
    _fd = other._fd;
    _nickname = other._nickname;
    _username = other._username;
    _realname = other._realname;
    _buffer = other._buffer;
  	_outBuffer = other._outBuffer;
    _passOk = other._passOk;
    _registered = other._registered;
  }
  return (*this);
}

// Destroys the client object and releases its managed C++ members.
Client::~Client()
{
}

// Returns the client's socket file descriptor.
int Client::getFd() const
{
	return (_fd);
}

// Returns the client's current IRC nickname.
std::string Client::getNickname() const
{
	return (_nickname);
}

// Checks whether the client has completed IRC registration.
bool Client::isRegistered() const
{
	return (_registered);
}

// Appends newly received TCP data to the client's input buffer.
void Client::appendBuffer(const std::string& data)
{
	_buffer += data;
}

// Extracts one complete CRLF-terminated IRC message
// and keeps any remaining data in the input buffer.
std::string Client::extractMessage()
{
	size_t		pos;
	std::string	message;

	pos = _buffer.find("\r\n");
	if (pos == std::string::npos)
		return ("");
	message = _buffer.substr(0, pos);
	_buffer.erase(0, pos + 2);
	return (message);
}

// Returns the client's registered username.
std::string Client::getUsername() const
{
	return (_username);
}

// Returns the client's registered real name.
std::string Client::getRealname() const
{
	return (_realname);
}

// Returns whether the client has provided the correct server password.
bool Client::isPassOk() const
{
	return (_passOk);
}

// Updates the client's IRC nickname.
void Client::setNickname(const std::string& nickname)
{
	_nickname = nickname;
}

// Updates the client's username.
void Client::setUsername(const std::string& username)
{
	_username = username;
}

// Updates the client's real name.
void Client::setRealname(const std::string& realname)
{
	_realname = realname;
}

// Updates the client's password authentication status.
void Client::setPassOk(bool value)
{
	_passOk = value;
}

// Updates the client's IRC registration status.
void Client::setRegistered(bool value)
{
	_registered = value;
}

// Appends outgoing data to the client's output buffer.
void Client::appendOutput(const std::string& data)
{
	_outBuffer += data;
}

// Returns a mutable reference to the client's pending output buffer.
std::string& Client::getOutputBuffer()
{
	return (_outBuffer);
}

// Checks whether the client has pending data to send.
bool Client::hasOutput() const
{
	return (_outBuffer.empty() == false);
}

// Removes the specified number of bytes from the output buffer
// after they have been successfully sent.
void Client::eraseOutput(size_t len)
{
	_outBuffer.erase(0, len);
}