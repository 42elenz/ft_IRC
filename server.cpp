#include "server.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <sstream>
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

	this->socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socketfd < 0)
	{
		perror("Socket");
		return;
	}
	if (fcntl(this->socketfd, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("Fcntl");
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
		if (poll(&(*pfds.begin()), pfds.size(), -1) == -1)
		{
			perror("Poll");
			return;
		}
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
					bufcnt = recv(pfds[i].fd, buf, 511, 0);
					buf[bufcnt] = '\0';
					Recieve();
					pfds[i].events = 0;
				}
			}
			i++;
		}
	}
}

void Server::Recieve()
{
	std::stringstream stream;
	std::string str;

	stream << buf;
	while (std::getline(stream, str, '\n'))
	{
		Command(str);
	}

}

void Server::Command(std::string cmd)
{
	std::stringstream stream;
	std::string str;

	stream << cmd;
	if (str == "CAP")
		std::cout << "C: " << str << std::endl;
	else if (str == "NICK")
		std::cout << "N: " << str << std::endl;
	else if (str == "USER")
		std::cout << "U: " << str << std::endl;
}