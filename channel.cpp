#include "channel.hpp"

Channel::Channel(User *first_nick)
{
	users.insert(std::make_pair<User *, bool>(first_nick, true));
}

Channel::~Channel()
{

}

void Channel::addUser(User *user)
{
	users.insert(std::make_pair<User *, bool>(user, false));
}

void Channel::removeUser(User *user)
{
	users.erase(user);
}

void Channel::sendToAll(const std::string &msg)
{
	std::map<User *, bool>::iterator iter;

	iter = users.begin();
	while (iter != users.end())
	{
		iter->first->sendMsg(msg);
		iter++;
	}
}

int Channel::getUserCount()
{
	return (users.size());
}