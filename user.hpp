#pragma once

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <sys/socket.h>

#include "channel.hpp"

class Channel;

class User
{
	private:
		std::string													nick_;
		std::string													user_;
		std::string													real_;
		std::string													fail_;
		const int													&fd_;
		std::vector<std::map<std::string, Channel>::pointer>		channels;
		bool														passwd;
	public:
		User(const int &fd);
		~User();
		std::string getNick();
		void setNick(std::string nick);
		std::string getUser();
		void setUser(std::string user);
		std::string getReal();
		void setReal(std::string real);
		std::string getFail();
		void setFail(std::string nick);
		void joinChannel(std::map<std::string, Channel>::pointer channel_ptr);
		void leaveChannel(std::map<std::string, Channel>::pointer channel_ptr);
		void sendMsg(const std::string &msg);
		std::map<std::string, Channel>::pointer findChannel(const std::string &channel_str);
		void setPasswd();
		bool hasPasswd();
};