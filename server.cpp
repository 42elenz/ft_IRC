#include "server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fcntl.h>
#include <arpa/inet.h>
#include <poll.h>
#include <stdio.h>
#include <unistd.h>

Server::Server(unsigned int port) : port(port), socketfd(-1)
{

}

Server::~Server()
{
	if (socketfd != -1)
		close(socketfd);
}

void Server::StartServer()
{
	struct sockaddr_in addr;
	std::vector<struct pollfd> pfds;
	unsigned int i;
	char buf[512];

	this->socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socketfd < 0)
	{
		perror("Socket creation");
		return;
	}
	if (fcntl(this->socketfd, F_SETFL, O_NONBLOCK) == -1)
	{
		std::cerr << "Could not set socket to non-blocking" << std::endl;
		return;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(this->port);
	if (bind(this->socketfd, (const struct sockaddr*) &addr, sizeof(addr)) == -1)
	{
		perror("Bind");
		return;
	}

	if (listen(this->socketfd, SOMAXCONN) == -1)
	{
		perror("Listen");
		return;
	}
	pfds.push_back(pollfd());
	pfds[0].fd = this->socketfd;
	pfds[0].events = POLLIN;
	while(1)
	{
		i = 0;
		poll(&(*pfds.begin()), pfds.size(), -1);
		while (i < pfds.size())
		{
			if (pfds[i].revents == POLLIN)
			{
				if (pfds[i].fd == this->socketfd)
				{
					pfds.push_back(pollfd());
					pfds.back().fd = accept(this->socketfd, NULL, NULL);
					pfds.back().events = POLLIN;
				}
				else
				{
					recv(pfds[i].fd, buf, 512, 0);
					pfds.size();
					pfds[i].events = 0;
				}
			}
			i++;
		}
	}
}