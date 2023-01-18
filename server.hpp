#pragma once

#include <vector>
#include <list>
#include <map>
#include "user.hpp"
#include "channel.hpp"

class Server
{
	private:
		unsigned int						port;
		std::string							password;
		int									socketfd;
		std::vector<struct pollfd>			pfds;
		std::list<User>						users;
		std::map<std::string, Channel>		channels;

	public:
		Server(unsigned int port, std::string password);
		~Server();
		void StartServer();

	private:
		void RemoveUser(std::vector<struct pollfd>::iterator &pfds_iter, std::list<User>::iterator &users_iter);
		std::string Recieve(const int &fd, User &user);
		void Send(const int &fd, std::string &reply);
		void RemoveUser(User &user);
		std::string Command(const std::string &cmd, User &user);
		std::string PassCmd(std::stringstream &stream, User &user);
		std::string NickCmd(std::stringstream &stream, User &user);
		std::string UserCmd(std::stringstream &stream, User &user);
		std::string OuitCmd();
		std::string JoinCmd(std::stringstream &stream, User &user);
		std::string PingCmd();
		std::string PartCmd(std::stringstream &stream, User &user);
		std::string ModeCmd(std::stringstream &stream, User &user);
		std::string TopicCmd(std::stringstream &stream, User &user);
		std::string ListCmd(std::stringstream &stream, User &user);
		std::string KickCmd(std::stringstream &stream, User &user);
		std::string PrivmsgCmd(std::stringstream &stream, User &user);
		std::string NoticeCmd(std::stringstream &stream, User &user);
};