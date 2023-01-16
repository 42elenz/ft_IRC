#pragma once

#include <map>

#include "user.hpp"

class User;

class Channel
{
	private:
		std::map<User *, bool>	users;
		std::string				topic_;
		bool					only_op_can_change_topic_;
	public:
		Channel(User *first_user);
		~Channel();
		void addUser(User *user);
		void removeUser(User *user);
		int getUserCount();
		void setTopic(const std::string &topic);
		std::string getTopic();
		void sendToAll(const std::string &msg);
		bool isUserAllowedToChangeTopic(User *user);
};