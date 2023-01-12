#include <sstream>
#include <iostream>

#include "server.hpp"

int main(int cnt, char **args)
{
	unsigned int port;

	if (cnt != 3)
	{
		std::cerr << "Please provide port and password:\n./ircserv <port> <password>" << std::endl;
		return (0);
	}
	std::stringstream portstream(args[1]);
	portstream >> port;

	if (portstream.fail() || !portstream.eof() || port > 65535)
		std::cerr << "Please provide a valid port:\n./ircserv <port> <password>" << std::endl;

	Server server(port);
	server.StartServer();
	return(0);
}