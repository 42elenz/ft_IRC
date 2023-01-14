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

Server::Server(unsigned int port, std::string password) : port(port), password(password), socketfd(-1)
{

}

Server::~Server()
{
	if (socketfd != -1)
		close(socketfd);
	std::cout << "Server terminated." << std::endl;
}

void Server::StartServer()
{
	struct sockaddr_in addr;
	std::string reply;
	unsigned int i;

	std::cout << "Server started." << std::endl;
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
	users.push_back(User());
	pfds[0].fd = this->socketfd;
	pfds[0].events = POLLIN;
	while(1)
	{
		std::cout << "Poll." << std::endl;
		if (poll(&(*pfds.begin()), pfds.size(), -1) == -1)
		{
			perror("Poll");
			return;
		}
		if (pfds[0].revents == POLLIN)
		{
			pfds.push_back(pollfd());
			pfds.back().fd = accept(this->socketfd, NULL, NULL);
			if (fcntl(pfds.back().fd, F_SETFL, O_NONBLOCK) == -1)
			{
				perror("Fcntl");
				return;
			}
			pfds.back().events = POLLIN;
			users.push_back(User());
			std::cout << "Client file descriptor established." << std::endl;
		}
		else
		{
			i = 1;
			while (i < pfds.size())
			{
				if (pfds[i].revents == POLLIN)
				{
						reply = Recieve(pfds[i].fd, users[i]);
						if (reply != "")
							Send(pfds[i].fd, reply);
				}
				else
				{
					RemoveUser(i);
				}
				i++;
			}
		}
	}
}

void Server::RemoveUser(const int &index)
{
	std::cout << "User '" << users[index].getUser() << "' (Nick '" << users[index].getNick() << "') removed from server." << std::endl;
	close(pfds[index].fd);
	pfds.erase(pfds.begin() + index);
	users.erase(users.begin() + index);
}

std::string Server::Recieve(const int &fd, User &user)
{
	std::stringstream stream;
	std::string str;
	std::string ret;
	char buf[4096];
	int bufcnt;

	ret = "";
	bufcnt = recv(fd, buf, 511, 0);
	buf[bufcnt] = '\0';
	std::cout << "Buffer: " << buf << std::endl;
	stream << buf;
	while (std::getline(stream, str, '\n'))
	{
		ret = ret + Command(str, user);
	}
	return (ret);
}

void Server::Send(const int &fd, std::string &reply)
{
	std::cout << "_________________\nOutgoing commands\n\n" << reply << "_________________\n" << std::endl;
	send(fd, reply.c_str(), reply.length(), 0);
}

std::string Server::Command(const std::string &cmd, User &user)
{
	std::stringstream stream;
	std::string str;

	stream << cmd;
	std::getline(stream, str, ' ');
	if (str[0] == ':')
		std::getline(stream, str, ' ');
	if (str == "PASS")
		return PassCmd(stream, user);
	else if (str == "NICK")
		return NickCmd(stream, user);
	else if (str == "USER")
		return UserCmd(stream, user);
	else if (str == "QUIT")
		return OuitCmd(stream, user);
	else if (str == "JOIN")
		return JoinCmd(stream, user);
	else if (str == "PING")
		return PingCmd(stream, user);
	else if (str == "PART")
		return PartCmd(stream, user);
	else if (str == "MODE")
		return ModeCmd(stream, user);
	else if (str == "TOPIC")
		return TopicCmd(stream, user);
	else if (str == "LIST")
		return ListCmd(stream, user);
	else if (str == "KICK")
		return KickCmd(stream, user);
	else if (str == "PRIVMSG")
		return PrivmsgCmd(stream, user);
	else if (str == "NOTICE")
		return NoticeCmd(stream, user);
	return ("");
}

std::string Server::PassCmd(std::stringstream &stream, User &user)
{
	std::string str;

	(void) user;
	if (std::getline(stream, str, ' ') == NULL)
	{
		std::cout << "Recieved incorrect password command!"<< std::endl;
		return ("");
	}
	str.erase(str.find_last_not_of('\r') + 1, std::string::npos);
	if (str == password)
		std::cout << "Recieved correct password. " << str << std::endl;
	else
		std::cout << "Recieved incorrect password. " << str << " vs " << password << std::endl;
	return ("");
}

std::string Server::NickCmd(std::stringstream &stream, User &user)
{
	std::string str;

	if (std::getline(stream, str, ' ') != NULL)
	{
		str.erase (str.find_last_not_of('\r') + 1, std::string::npos);
		user.setNick(str);
		std::cout << "Nick '" << str << "' set." << std::endl;
		return ("");
	}
	std::cout << "Nick command incorrect!" << std::endl;
	return ("");
}

std::string Server::UserCmd(std::stringstream &stream, User &user)
{
	std::string str;

	if (std::getline(stream, str, ':') != NULL && std::getline(stream, str, '\r') != NULL)
	{
		user.setUser(str);
		std::cout << "User '" << str << "' set." << std::endl;
		return (":localhost 001 " + user.getNick() + " :Welcome to the Internet Relay Network "
				+ user.getNick() + "!" + user.getUser() + "@localhost\r\n002 "
				+ user.getNick() + "\r\n003 " + user.getNick() + "\r\n004 " + user.getNick() + "\r\n");
	}
	std::cout << "User command incorrect!" << std::endl;
		return ("");
}

std::string Server::OuitCmd(std::stringstream &stream, User &user)
{
	(void) stream;
	(void) user;
	std::cout << "Not Implemented! " << std::endl;
	return "";
}

std::string Server::JoinCmd(std::stringstream &stream, User &user)
{
	(void) stream;
	(void) user;
	std::cout << "Not Implemented! " << std::endl;
	return "";
}

std::string Server::PingCmd(std::stringstream &stream, User &user)
{
	(void) stream;
	(void) user;
	std::cout << "Ping command recieved." << std::endl;
	return "PONG localhost\r\n";
}

std::string Server::PartCmd(std::stringstream &stream, User &user)
{
	(void) stream;
	(void) user;
	std::cout << "Not Implemented! " << std::endl;
	return "";
}

std::string Server::ModeCmd(std::stringstream &stream, User &user)
{
	(void) stream;
	(void) user;
	std::cout << "Not Implemented! " << std::endl;
	return "";
}

std::string Server::TopicCmd(std::stringstream &stream, User &user)
{
	(void) stream;
	(void) user;
	std::cout << "Not Implemented! " << std::endl;
	return "";
}

std::string Server::ListCmd(std::stringstream &stream, User &user)
{
	(void) stream;
	(void) user;
	std::cout << "Not Implemented! " << std::endl;
	return "";
}

std::string Server::KickCmd(std::stringstream &stream, User &user)
{
	(void) stream;
	(void) user;
	std::cout << "Not Implemented! " << std::endl;
	return "";
}

std::string Server::PrivmsgCmd(std::stringstream &stream, User &user)
{
	(void) stream;
	(void) user;
	std::cout << "Not Implemented! " << std::endl;
	return "";
}

std::string Server::NoticeCmd(std::stringstream &stream, User &user)
{
	(void) stream;
	(void) user;
	std::cout << "Not Implemented! " << std::endl;
	return "";
}
