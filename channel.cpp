#include "channel.hpp"

Channel::Channel(User *first_nick) : topic_(":"), only_op_can_change_topic_(false)
{
	users.insert(std::make_pair<User *, bool>(first_nick, true));

}

Channel::~Channel()
{

}

bool Channel::addUser(User *user)
{
	return (users.insert(std::make_pair<User *, bool>(user, false)).second);
}

void Channel::removeUser(User *user)
{
	users.erase(user);
}

std::map<User *, bool>::pointer Channel::findUser(std::string user)
{
	std::map<User *, bool>::iterator iter;

	iter = users.begin();
	while (iter != users.end())
	{
		if (user == iter->first->getNick())
			return (&(*iter));
		iter++;
	}
	return (NULL);
}

void Channel::sendToAll(const std::string &msg, User *user)
{
	std::map<User *, bool>::iterator iter;

	iter = users.begin();
	while (iter != users.end())
	{
		std::cout << iter->first->getNick() << "!! ";
		if (user != iter->first)
			iter->first->sendMsg(msg);
		else
			std::cout << " except\n";
		++iter;
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

void Channel::setFlagT(const bool &flag)
{
	only_op_can_change_topic_ = flag;
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

bool Channel::isUserChannelOperator(User *user)
{
	std::map<User *, bool>::iterator iter;

	iter = users.find(user);
	if (iter == users.end())
		return (false);
	return (iter->second);
}

void Channel::setUserChannelOperator(User *user, const bool &value)
{
	std::map<User *, bool>::iterator iter;

	iter = users.find(user);
	iter->second = value;
}

std::string Channel::nameReply(const std::string &channel_name, const std::string &nick)
{
	std::map<User *, bool>::iterator iter;
	std::string ret;

	if (topic_ == ":")
		ret = "331 " + channel_name + " :No topic is set\r\n";
	else
		ret = "332 " + channel_name + " " + topic_ + "\r\n";
	ret = ret + "353 " + nick + " = " + channel_name + " :";
	iter = users.begin();
	while (1)
	{
		if (iter->second == true)
			ret = ret + "@" + iter->first->getNick();
		else
		ret = ret + iter->first->getNick();
		++iter;
		if (iter == users.end())
			break;\
		ret = ret + " ";
	}
	ret = ret + "\r\n366 " + nick + " " + channel_name + " :End of /NAMES list\r\n";
	return (ret);
}