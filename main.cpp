#include <sstream>
#include <iostream>

#include "server.hpp"


/* 
Todo:
Error in nc -> when not authenticated and then CTRL-C -> endless loop
Still reachable leaks - Because we have "terminating server?":
 Process terminating with default action of signal 2 (SIGINT)
==37561==    at 0x4B8846E: recv (recv.c:28)
==37561==    by 0x10C738: Server::Recieve[abi:cxx11](int const&, User&) (server.cpp:146)
==37561==    by 0x10C2E4: Server::StartServer() (server.cpp:88)
==37561==    by 0x11933E: main (main.cpp:23)
 */
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
	{
		std::cerr << "Please provide a valid port:\n./ircserv <port> <password>" << std::endl;
		return (0);
	}
	Server server(port, args[2]);
	server.StartServer();
	return(0);
}