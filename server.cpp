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
	struct sockaddr_in						addr;
	std::string								reply;
	std::vector<struct pollfd>::iterator	pfd_iter;
	std::list<User>::iterator				user_iter;

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
			users.push_back(User(pfds.back().fd));
			std::cout << "Client file descriptor established." << std::endl;
		}
		else
		{
			pfd_iter = ++(pfds.begin());
			user_iter = users.begin();

			while (pfd_iter != pfds.end())
			{
				if (pfd_iter->revents == POLLIN)
				{
					reply = Recieve(pfd_iter->fd, *user_iter);
					if (reply == "USERREMOVED")
						continue;
					if (reply != "")
						Send(pfd_iter->fd, reply);
				}
				else if (pfd_iter->revents != 0)
				{
					RemoveUser(pfd_iter, user_iter);
					continue;
				}
				++pfd_iter;
				++user_iter;
			}
		}
	}
}

void Server::RemoveUser(std::vector<struct pollfd>::iterator &pfd_iter, std::list<User>::iterator &user_iter)
{
	std::cout << "User '" << user_iter->getUser() << "' (Nick '" << user_iter->getNick() << "') removed from server." << std::endl;
	close(pfd_iter->fd);
	pfds.erase(pfd_iter);
	users.erase(user_iter);
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
		str.erase(str.find_last_not_of('\r') + 1, std::string::npos);
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
	if (str == "PASS" && user.hasPasswd() == false)
	{
		str = PassCmd(stream, user);
		if (str != "")
			user.sendMsg(str);
	}
	if (user.hasPasswd() == false)
		return("");
	if (str[0] == ':')
		std::getline(stream, str, ' ');
	if (str == "PASS")
		return ("462 :Unauthorized command (already registered)");
	else if (str == "NICK")
		return NickCmd(stream, user);
	else if (str == "USER")
		return UserCmd(stream, user);
	else if (str == "QUIT")
		return OuitCmd(stream, user);
	else if (str == "JOIN")
		return JoinCmd(stream, user);
	else if (str == "PING")
		return PingCmd();
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
		return ("461 PASS :Not enough parameters\r\n");
	std::cout << str << " " << password << std::endl;
	if (str == password)
	{
		user.setPasswd();
		return ("");
	}
	return ("464 :Password incorrect\r\n");
}

std::string Server::NickCmd(std::stringstream &stream, User &user)
{
	std::string					str;
	std::list<User>::iterator	user_iter;
	std::string					old_nick;

	if (std::getline(stream, str, ' ') != NULL)
	{
		user_iter = users.begin();
		while (user_iter != users.end())
		{
			if (user_iter->getNick() == str)
				return ("433 " + str + " :Nickname is already in use\r\n");
				++user_iter;
		}
		old_nick = user.getNick();
		user.setNick(str);
		if (old_nick == "")
			return ("");
		return (":" + old_nick + " NICK " + str + "\r\n" );
	}
	return ("431 :No nickname given\r\n");
}

std::string Server::UserCmd(std::stringstream &stream, User &user)
{
	std::string str;

	if (std::getline(stream, str, ':') != NULL && std::getline(stream, str, '\0') != NULL)
	{
		user.setUser(str);
		std::cout << "User '" << str << "' set." << std::endl;
		return (":localhost 001 " + user.getNick() + " :Welcome to the Internet Relay Network "
				+ user.getNick() + "!" + user.getUser() + "@localhost\r\n");
	}
	std::cout << "User command incorrect!" << std::endl;
		return ("");
}

std::string Server::OuitCmd(std::stringstream &stream, User &user)
{
	(void) stream;
	std::cout << "User " << user.getUser() << " '" << user.getNick() << "'" << " quit the server." << std::endl;
	return ("ERROR :terminating client connection\r\n");
}

std::string Server::JoinCmd(std::stringstream &stream, User &user)
{
	std::string str;
	std::map<std::string, Channel>::pointer channel_ptr;

	if (std::getline(stream, str, ',') != NULL && (str[0] == '#' || str[0] == '&' || str[0] == '!' || str[0] == '+'))
	{
		if (channels.count(str) == 0)
		{
			channel_ptr = &(*(channels.insert(std::make_pair<const std::string, Channel>(str, Channel(&user))).first));
			if(str[0] == '+')
				channel_ptr->second.setFlagT(true);
		}
		else
		{
			channel_ptr = &(*channels.find(str));
			channel_ptr->second.addUser(&user);
		}
		channel_ptr->second.sendToAll(":" + user.getNick() + " JOIN " + str + "\r\n");
		user.joinChannel(channel_ptr);
		return ( channel_ptr->second.nameReply(channel_ptr->first, user.getNick()) + "366 " + user.getNick() + " " + str + " :End of /NAMES list\r\n");
	}
	std::cout << "Not Implemented! " << std::endl;
	return "";
}

std::string Server::PingCmd()
{
	return "PONG localhost\r\n";
}

std::string Server::PartCmd(std::stringstream &stream, User &user)
{
	std::string str;
	std::string ret;
	std::stringstream channel_stream;
	std::map<std::string, Channel>::iterator	channel_iter;

	if (std::getline(stream, str, ' ') != NULL && (str[0] == '#' || str[0] == '&' || str[0] == '!' || str[0] == '+'))
	{
		ret = "";
		channel_stream << str;
		while (std::getline(channel_stream, str, ',') != NULL && (str[0] == '#' || str[0] == '&' || str[0] == '!' || str[0] == '+'))
		{
			channel_iter = channels.find(str);
			if (channel_iter == channels.end())
				ret = ret + user.getUser() + " " + str + " :No such channel\r\n";
			else
			{
				if (std::getline(stream, str, '\0') && str[0] == ':')
					channel_iter->second.sendToAll(":" + user.getNick() + " PART " + channel_iter->first + " " + str + "\r\n");
				else
					channel_iter->second.sendToAll(":" + user.getNick() + " PART " + channel_iter->first + "\r\n");
				if (channel_iter->second.getUserCount() == 1)
					channels.erase(channel_iter);
				else
				{
					channel_iter->second.removeUser(&user);
					user.leaveChannel(&(*channel_iter));
				}
			}
		}
		return (ret);
	}
	return ("461 PART :Not enough parameters\r\n");
}

std::string Server::ModeCmd(std::stringstream &stream, User &user)
{
	std::string 								str;
	std::map<std::string, Channel>::pointer		channel_ptr;

	if (std::getline(stream, str, ' ') != NULL && (str[0] == '#' || str[0] == '&' || str[0] == '!' || str[0] == '+'))
	{
		channel_ptr = user.findChannel(str);
		if (str[0] == '+')
			return ("477 " + str + " :Channel doesn't support modes\r\n");
		if (channel_ptr == NULL || channel_ptr->second.isUserChannelOperator(&user) == false)
			return ("482 " + str + " :You're not channel operator\r\n");
		while (std::getline(stream, str, ' ') != NULL)
		{
			if (str == "+t")
				channel_ptr->second.setFlagT(true);
			else if (str == "-t")
				channel_ptr->second.setFlagT(false);
			else if (str == "-o" && std::getline(stream, str, ' ') != NULL)
			{
				if (str == user.getNick())
					channel_ptr->second.setUserChannelOperator(&user, false); // part missing TODO
			}
			else if (str == "+o" && std::getline(stream, str, ' ') != NULL && channel_ptr->second.isUserChannelOperator(&user))
			{
				channel_ptr->second.setUserChannelOperator(&user, true); // Incorrect TODO
			}
		}
	}
	return ("461 MODE :Not enough parameters\r\n");
}

std::string Server::TopicCmd(std::stringstream &stream, User &user)
{
	std::string str;
	std::map<std::string, Channel>::pointer channel_ptr;

	if (std::getline(stream, str, ' ') != NULL && (str[0] == '#' || str[0] == '&' || str[0] == '!' || str[0] == '+'))
	{
		channel_ptr = user.findChannel(str);
		if (channel_ptr == NULL)
			return ("442 " + str + " :You're not on that channel\r\n");
		if (std::getline(stream, str, '\0') == NULL)
		{
			if (channel_ptr->second.getTopic()  == ":")
				return ("331 " + channel_ptr->first + " :No topic is set\r\n");
			else
				return ("332 " + channel_ptr->first + " " + channel_ptr->second.getTopic() + "\r\n");
		}
		else
		{
			if (channel_ptr->second.isUserAllowedToChangeTopic(&user) == false)
				return ("482 " + channel_ptr->first + " :You're not channel operator\r\n");
			channel_ptr->second.setTopic(str);
			channel_ptr->second.sendToAll(":" + user.getNick() + " TOPIC " + channel_ptr->first + " " + channel_ptr->second.getTopic() + "\r\n");
			return ("");
		}
	}
	return ("461 PART :Not enough parameters\r\n");
}

std::string Server::ListCmd(std::stringstream &stream, User &user)
{
	std::string ret;
	(void) stream;
	(void) user;
	ret = "321 Channel :Users Name\r\n";
	std::map<std::string, Channel>::iterator it = channels.begin();
	for (; it != channels.end(); it++)
		ret += "322 " + it->first + " ";
	std::cout << "Not Implemented! " << std::endl;
	return "";
}

std::string Server::KickCmd(std::stringstream &stream, User &user)
{
	(void) user;
	std::string channel;
	std::string users;
	std::string ret;
	if ((std::getline(stream, channel, ' ') ==  NULL) || (std::getline(stream, users, ' ') == NULL))
		return ("461 KICK :Not enough parameters\r\n");
	stream >> channel;
	for(std::getline(stream, channel, ',') != NULL)
	{
		std::map<std::string, Channel>::iterator iter;
		iter = channels.find(channel);
		if (iter == NULL)
			ret = "403 " + channel + " :No such channel\r\n";
		else
		{
			iter->second. 
		}
	}

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
