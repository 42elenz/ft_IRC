#pragma once

#include <string>

class User
{
	private:
		std::string nick_;
		std::string user_;
	public:
		User();
		~User();
		std::string getNick();
		void setNick(std::string nick);
		std::string getUser();
		void setUser(std::string user);
};