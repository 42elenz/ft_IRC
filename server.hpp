#pragma once

#include <vector>
#include "user.hpp"

class Server
{
	private:
		unsigned int				port;
		std::string					password;
		int							socketfd;
		std::vector<struct pollfd>	pfds;
		std::vector<User>			users;

	public:
		Server(unsigned int port, std::string password);
		~Server();
		void StartServer();

	private:
		void RemoveUser(const int &index);
		std::string Recieve(const int &fd, User &user);
		void Send(const int &fd, std::string &reply);
		std::string Command(const std::string &cmd, User &user);
		std::string PassCmd(std::stringstream &stream, User &user);
		std::string NickCmd(std::stringstream &stream, User &user);
		std::string UserCmd(std::stringstream &stream, User &user);
		std::string OuitCmd(std::stringstream &stream, User &user);
		std::string JoinCmd(std::stringstream &stream, User &user);
		std::string PingCmd(std::stringstream &stream, User &user);
		std::string PartCmd(std::stringstream &stream, User &user);
		std::string ModeCmd(std::stringstream &stream, User &user);
		std::string TopicCmd(std::stringstream &stream, User &user);
		std::string ListCmd(std::stringstream &stream, User &user);
		std::string KickCmd(std::stringstream &stream, User &user);
		std::string PrivmsgCmd(std::stringstream &stream, User &user);
		std::string NoticeCmd(std::stringstream &stream, User &user);
};