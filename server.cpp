#include "server.hpp"
#include <sys/socket.h>
#include <iostream>

Server::Server(unsigned int port):port(port)
{

}

Server::~Server()
{

}

void Server::StartServer()
{
	this->socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socketfd < 0)
		std::cerr << "SOCKET FAILED TO INIATE";
	
	
}