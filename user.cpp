#include "user.hpp"

User::User(const int &fd) : nick_(), user_(), fd_(fd), passwd(false)
{

}

User::~User()
{

}

void User::closeFd()
{
	close(fd_);
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

std::string User::getReal()
{
	return (real_);
}

void User::setReal(std::string real)
{
	real_ =  real;
}

std::string User::getFail()
{
	return (fail_);
}

void User::setFail(std::string fail)
{
	fail_ =  fail;
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

std::map<std::string, Channel>::pointer User::findChannel(const std::string &channel_str)
{
	std::vector<std::map<std::string, Channel>::pointer>::iterator iter;

	iter = channels.begin();
	while(iter != channels.end())
	{
		if((*iter)->first == channel_str)
			return (*iter);
		{
		}
		++iter;
	}
	return(NULL);
}

std::vector<std::map<std::string, Channel>::pointer>::iterator User::get_channels_begin()
{
	std::vector<std::map<std::string, Channel>::pointer>::iterator iter;
	iter = channels.begin();
	return (iter);
}

std::vector<std::map<std::string, Channel>::pointer>::iterator User::get_channels_end()
{
	std::vector<std::map<std::string, Channel>::pointer>::iterator iter;
	iter = channels.end();
	return (iter);
}
void User::setPasswd()
{
	passwd = true;
}

bool User::hasPasswd()
{
	return (passwd);
}