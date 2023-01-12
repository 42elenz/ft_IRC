#include "server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fcntl.h>
#include <arpa/inet.h>
#include <poll.h>

Server::Server(unsigned int port):port(port)
{

}

Server::~Server()
{

}

void Server::StartServer()
{
	struct sockaddr_in addr;
	struct pollfd pfds[1];

	this->socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socketfd < 0)
		std::cerr << "Socket failed to initiate" << std::endl;
	if (fcntl(this->socketfd, F_SETFL, O_NONBLOCK))
		std::cerr << "Could not set socket to non-blocking" << std::endl;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("localhost");
	addr.sin_port = htons(this->port);
	if (!bind(this->socketfd, (sockaddr*) &addr, sizeof(addr)))
		std::cerr << "Could not bind to socket" << std::endl;

	pfds[0].fd = this->socketfd;
	pfds[0].events = POLLIN;
	poll(pfds, 1, 5 * 60 * 1000);
}