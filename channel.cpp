#include "channel.hpp"

Channel::Channel(User *first_nick) : topic_(":"), only_op_can_change_topic_(false)
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

void Channel::setTopic(const std::string &topic)
{
	topic_ = topic;
}

std::string Channel::getTopic()
{
	return (topic_);
}


bool Channel::isUserAllowedToChangeTopic(User *user)
{
	std::map<User *, bool>::iterator iter;

	if (only_op_can_change_topic_ == false)
		return (true);
	iter = users.find(user);
	if (iter == users.end())
		return (false);
	return (iter->second);
}