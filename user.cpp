#include "user.hpp"

User::User(const int &fd) : nick_(), user_(), fd_(fd)
{

}

User::~User()
{

}

std::string User::getNick()
{
	return (nick_);
}

void User::setNick(std::string nick)
{
	nick_ = nick;
}

std::string User::getUser()
{
	return (user_);
}

void User::setUser(std::string user)
{
	user_ =  user;
}

void User::joinChannel(std::map<std::string, Channel>::pointer channel_ptr)
{
	channels.push_back(channel_ptr);
}

void User::leaveChannel(std::map<std::string, Channel>::pointer channel_ptr)
{
	std::vector<std::map<std::string, Channel>::pointer>::iterator iter;

	iter = channels.begin();
	while(iter != channels.end())
	{
		if(*iter == channel_ptr)
		{
			channels.erase(iter, ++iter);
			return ;
		}
		++iter;
	}
}

void User::sendMsg(const std::string &msg)
{
	std::cout << msg;
	send(fd_, msg.c_str(), msg.length(), 0);
}