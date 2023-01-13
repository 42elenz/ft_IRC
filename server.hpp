#pragma once

#include <vector>
#include "user.hpp"

class Server
{
	private:
		unsigned int port;
		char buf[512];
		int bufcnt;
		// std::vector<User> users;
		int socketfd;

	public:
		Server(unsigned int port);
		~Server();
		void StartServer();
		void Recieve();
		void Command(std::string cmd);
		void NickCommand();
};