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
		void setFlagT(const bool &flag);
		bool isUserAllowedToChangeTopic(User *user);
		bool isUserChannelOperator(User *user);
		void setUserChannelOperator(User *user, const bool &value);
		std::string nameReply(const std::string &channel_name, const std::string &nick);
};