#pragma once

#include <vector>
#include "user.hpp"

class Server
{
	private:
		unsigned int port;
		// std::vector<User> users;
		int socketfd;

	public:
		Server(unsigned int port);
		~Server();
		void StartServer();
};