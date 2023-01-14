#include "user.hpp"

User::User() : nick_(), user_()
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
