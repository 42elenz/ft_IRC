#pragma once

#include <map>

#include "user.hpp"

class User;

class Channel
{
	private:
		std::map<User *, bool> users;
	public:
		Channel(User *first_user);
		~Channel();
		void addUser(User *user);
		void removeUser(User *user);
		int getUserCount();
		void sendToAll(const std::string &msg);
};