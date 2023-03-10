#include "server.hpp"


Server::Server(unsigned int port, std::string password) : port(port), password(password), socketfd(-1)
{

}

Server::~Server()
{
	if (socketfd != -1)
		close(socketfd);
	std::list<User>::iterator user_iter = users.begin();
	while (user_iter != users.end())
	{
		user_iter->closeFd();
		++user_iter;
	}
	std::cout << "Server terminated." << std::endl;
}

void Server::StartServer()
{
	struct sockaddr_in						addr;
	std::string								reply;
	std::vector<struct pollfd>::iterator	pfd_iter;
	std::list<User>::iterator				user_iter;

	std::cout << "Server started." << std::endl;
	this->socketfd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socketfd < 0)
	{
		perror("Socket");
		return;
	}
	if (fcntl(this->socketfd, F_SETFL, O_NONBLOCK) == -1)
	{
		perror("Fcntl");
		return;
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(this->port);
	if (bind(this->socketfd, (const struct sockaddr*) &addr, sizeof(addr)) == -1)
	{
		perror("Bind");
		return;
	}
	if (listen(this->socketfd, SOMAXCONN) == -1)
	{
		perror("Listen");
		return;
	}
	pfds.push_back(pollfd());
	pfds[0].fd = this->socketfd;
	pfds[0].events = POLLIN;
	while(1)
	{
		std::cout << "POLL" << std::endl;
		if (pfds.empty() || poll(&(*pfds.begin()), pfds.size(), -1) <= 0)
		{
			perror("Poll");
			return;
		}
		if (pfds[0].revents == POLLIN)
		{
			pfds.push_back(pollfd());
			pfds.back().fd = accept(this->socketfd, NULL, NULL);
			if (fcntl(pfds.back().fd, F_SETFL, O_NONBLOCK) == -1)
			{
				perror("Fcntl");
				return;
			}
			pfds.back().events = POLLIN;
			users.push_back(User(pfds.back().fd));
			std::cout << "Client file descriptor established." << std::endl;
		}
		else
		{
			pfd_iter = ++(pfds.begin());
			user_iter = users.begin();

			while (pfd_iter != pfds.end())
			{
				if (pfd_iter->revents == POLLIN)
				{
					std::cout<< "POLLIN" <<std::endl;
					reply = Recieve(pfd_iter->fd, *user_iter);
					if (reply == "USERREMOVED")
					{
						continue;
					}
					if (reply != "")
						Send(pfd_iter->fd, reply);
				}
				else if (pfd_iter->revents != 0)
				{
					RemoveUser(pfd_iter, user_iter);
					continue;
				}
				++pfd_iter;
				++user_iter;
			}
		}
	}
}

void Server::RemoveUser(std::vector<struct pollfd>::iterator &pfd_iter, std::list<User>::iterator &user_iter)
{
	std::map<std::string, Channel>::iterator	channel_iter;
	std::map<std::string, Channel>::iterator	channel_iter_end;

	close(pfd_iter->fd);
	pfds.erase(pfd_iter);
	channel_iter = channels.begin();
	channel_iter_end = channels.end();
	while (channel_iter != channel_iter_end && (!channels.empty()))//to prevent seqfaults empty check
	{
		if (channel_iter->second.findUser(user_iter->getNick()) != NULL)
		{
			if (channel_iter->second.getUserCount() == 1)
			{
				channels.erase(channel_iter);
			}
			else
			{
				channel_iter->second.removeUser(&(*user_iter));
			}
		}
		if(!channels.empty()) //to prevent seqfaults empty check
			++channel_iter;
	}
	users.erase(user_iter);
}

std::string Server::Recieve(const int &fd, User &user)
{
	std::stringstream stream;
	std::string str;
	std::string ret;
	char buf[4096];
	int bufcnt;

	ret = "";
	bufcnt = recv(fd, buf, 511, 0);
	buf[bufcnt] = '\0';
	std::cout << "Buffer: " << buf;
	stream << buf;
	while (std::getline(stream, str, '\n'))
	{
		str.erase(str.find_last_not_of('\r') + 1, std::string::npos);
		ret = ret + Command(str, user);
	}
	return (ret);
}

void Server::Send(const int &fd, std::string &reply)
{
	std::cout << reply;
	send(fd, reply.c_str(), reply.length(), 0);
}

std::string Server::Command(const std::string &cmd, User &user)
{
	std::stringstream stream;
	std::string str;

	stream << cmd;
	std::getline(stream, str, ' ');
	if (str == "PASS" && user.hasPasswd() == false)
	{
		str = PassCmd(stream, user);
		if (str != "")
			user.sendMsg(str);
	}
	if (user.hasPasswd() == false)
		return("");
	if (str[0] == ':')
		std::getline(stream, str, ' ');
	if (str == "PASS")
		return ("462 :Unauthorized command (already registered)\r\n");
	else if (str == "NICK")
		return NickCmd(stream, user);
	else if (str == "USER")
		return UserCmd(stream, user);
	else if (str == "QUIT")
		return OuitCmd(stream, user);
	else if (str == "JOIN")
		return JoinCmd(stream, user);
	else if (str == "PING")
		return PingCmd();
	else if (str == "PART")
		return PartCmd(stream, user);
	else if (str == "MODE")
		return ModeCmd(stream, user);
	else if (str == "TOPIC")
		return TopicCmd(stream, user);
	else if (str == "LIST")
		return ListCmd(stream, user);
	else if (str == "KICK")
		return KickCmd(stream, user);
	else if (str == "PRIVMSG")
		return PrivmsgCmd(stream, user);
	else if (str == "NOTICE")
		return NoticeCmd(stream, user);
	return ("");
}

std::string Server::PassCmd(std::stringstream &stream, User &user)
{
	std::string str;

	(void) user;
	if (std::getline(stream, str, ' ') == NULL)
		return ("461 PASS :Not enough parameters\r\n");
	if (str == password)
	{
		user.setPasswd();
		return ("");
	}
	return ("464 PASS :Password incorrect\r\n");
}

std::string Server::NickCmd(std::stringstream &stream, User &user)
{
	std::string					str;
	std::list<User>::iterator	user_iter;
	std::string					old_nick;

	if (std::getline(stream, str, ' ') == NULL)
		return ("431 :No nickname given\r\n");
	user_iter = users.begin();
	while (user_iter != users.end())
	{
		if (user_iter->getNick() == str)
		{
			user.setFail(str);
			return ("433 " + str + " :Nickname is already in use\r\n");
		}
		++user_iter;
	}
	old_nick = user.getNick();
	user.setNick(str);
	if (old_nick != "")
	{
		user_iter = users.begin();
		while (user_iter != users.end())
		{
			user_iter->sendMsg(":" + old_nick + " NICK " + str + "\r\n");
			++user_iter;
		}
		return ("");
	}
	if (user.getFail() != "" && user.getNick() == "")
		old_nick = (":" + user.getFail() + " NICK " + str + "\r\n");
	if (user.getUser() == "")
		return ("");
	return (old_nick + ":localhost 001 " + user.getNick() + " :Welcome to the Internet Relay Network " + user.getNick() + "!" + user.getUser() + "@localhost\r\n");
}

std::string Server::UserCmd(std::stringstream &stream, User &user)
{
	std::string user_str;
	std::string real_str;

	if (user.getUser() != "" || user.getReal() != "")
		return ("462 :Unauthorized command (already registered)\r\n");
	if (std::getline(stream, user_str, ' ') == NULL || std::getline(stream, real_str, ' ') == NULL || std::getline(stream, real_str, ':') == NULL)
		return ("461 USER :Not enough parameters\r\n");
	std::getline(stream, real_str, '\0');
	user.setUser(user_str);
	user.setReal(real_str);
	if (user.getNick() == "")
		return ("");
	return (":localhost 001 " + user.getNick() + " :Welcome to the Internet Relay Network " + user.getNick() + "!" + user.getUser() + "@localhost\r\n");
}

std::string Server::OuitCmd(std::stringstream &stream, User &user)
{
	std::vector<std::map<std::string, Channel>::pointer>::iterator ch_iter;
	std::vector<std::map<std::string, Channel>::pointer>::iterator ch_iter_end;
	std::string 													pm_str;
	std::string 													msg;
	std::string														ret;
	std::stringstream												msg_stream;

	ch_iter = user.get_channels_begin();
	ch_iter_end = user.get_channels_end();
	std::getline(stream, msg);
	if (msg[1] == ':')
		msg = msg.substr(1);
	while(ch_iter != ch_iter_end)
	{
		pm_str = user.getNick() + " quitted server :" + msg;
		(*ch_iter)->second.sendToAll(":" + user.getNick() + " PRIVMSG " + (*ch_iter)->first + " " + pm_str + "\r\n", &user);
		ch_iter++;
	}
	ret += "ERROR :terminating client connection\r\n";
	return (ret);
}

std::string Server::JoinCmd(std::stringstream &stream, User &user)
{
	std::string str;
	std::string ret;
	std::string part_str;
	std::stringstream part;
	std::map<std::string, Channel>::pointer channel_ptr;
	std::vector<std::map<std::string, Channel>::pointer>::iterator iter;
	std::vector<std::map<std::string, Channel>::pointer>::iterator iter_end;

	if (std::getline(stream, str, ' ') == NULL)
		return ("461 JOIN :Not enough parameters\r\n");
	if (str[0] == '0')
	{
		iter = user.get_channels_begin();
		iter_end = user.get_channels_end();
		while(iter != iter_end)
		{
			part_str = (*iter)->first;
			part_str += " :WeeChat 3.8";
			part.clear();
			part << part_str;
			ret += PartCmd(part, user);
			iter++;
		}
		return (ret);
	}
	stream.clear();
	stream.str(str);
	while (std::getline(stream, str, ',') != NULL)
	{
		if (!(str[0] == '#' || str[0] == '&' || str[0] == '!' || str[0] == '+'))
			ret += "476 " + str + " :Bad Channel Mask\r\n";
		else
		{
			if (channels.count(str) == 0)
			{
				channel_ptr = &(*(channels.insert(std::make_pair<const std::string, Channel>(str, Channel(&user))).first));
				if(str[0] == '+')
					channel_ptr->second.setFlagT(true);
			}
			else
			{
				channel_ptr = &(*channels.find(str));
				if (channel_ptr->second.addUser(&user) == false)
					continue;
			}
			channel_ptr->second.sendToAll(":" + user.getNick() + " JOIN " + str + "\r\n", &user);
			user.joinChannel(channel_ptr);
			ret += ":" + user.getNick() + " JOIN " + str + "\r\n";
			ret += channel_ptr->second.nameReply(channel_ptr->first, user.getNick());
			ret += ":irc_bot NOTICE " + str + " :Welcome to the channel. Be polite and don't insult other users!\r\n";
		}
	}
	return (ret);
}

std::string Server::PingCmd()
{
	return "PONG localhost\r\n";
}

std::string Server::PartCmd(std::stringstream &stream, User &user)
{
	std::string str;
	std::string msg;
	std::string ret;
	std::map<std::string, Channel>::iterator	channel_iter;

	if (std::getline(stream, str, ' ') == NULL)
	{
		return ("461 PART :Not enough parameters\r\n");
	}
	if (std::getline(stream, msg, '\0') != NULL && msg[0] == ':')
		msg = " " + msg;
	else
		msg = "";
	stream.clear();
	stream << str;
	while (std::getline(stream, str, ' ') != NULL)
	{
		channel_iter = channels.find(str);
		if (channel_iter == channels.end())
			ret += user.getUser() + " " + str + " :No such channel\r\n";
		else
		{
			channel_iter->second.sendToAll(":" + user.getNick() + " PART " + channel_iter->first + msg + "\r\n", &user);
			ret += ":" + user.getNick() + " PART " + channel_iter->first + msg + "\r\n";
			user.leaveChannel(&(*channel_iter));
			if (channel_iter->second.getUserCount() == 1)
				channels.erase(channel_iter);
			else
				channel_iter->second.removeUser(&user);
		}
	}
	return (ret);
}

std::string Server::ModeCmd(std::stringstream &stream, User &user)
{
	std::map<std::string, Channel>::pointer		channel_ptr;
	std::string									channel_str;
	std::string									mode_str;
	std::string									param_str;
	std::map<User *, bool>::pointer				user_ptr;

	if (std::getline(stream, channel_str, ' ') == NULL || !(channel_str[0] == '#' || channel_str[0] == '&' || channel_str[0] == '!' || channel_str[0] == '+'))
		return ("461 MODE :Not enough parameters\r\n");
	channel_ptr = user.findChannel(channel_str);
	if (channel_ptr == NULL)
		return ("441 " + param_str + " " + channel_str + " :You aren't on the channel\r\n");
	if (channel_str[0] == '+')
		return ("477 " + channel_str + " :Channel doesn't support modes\r\n");
	if (std::getline(stream, mode_str, ' ') == NULL)
	{
		if (channel_ptr->second.isUserChannelOperator(&user) == false)
			return ("324 " + channel_str + " -m -o " + user.getNick() + "\r\n");
		else
			return ("324 " + channel_str + " -m +o " + user.getNick() + "\r\n");
	}
	if (!channel_ptr->second.isUserChannelOperator(&user))
		return("482 " + channel_str + " :You're not channel operator\r\n");
	while (1)
	{
		if (mode_str == "+t")
			channel_ptr->second.setFlagT(true);
		else if (mode_str == "-t")
			channel_ptr->second.setFlagT(false);
		else if (mode_str == "-o" && std::getline(stream, param_str, ' '))
		{
			user_ptr = channel_ptr->second.findUser(param_str);
			if (user_ptr == NULL)
				user.sendMsg("441 " + param_str + " " + channel_str + " :They aren't on that channel\r\n");
			else
			{
				user_ptr->second = false;
				channel_ptr->second.sendToAll(":localhost MODE " + channel_str + " " + mode_str + " " + param_str + "\r\n", &user);
				user.sendMsg(":localhost MODE " + channel_str + " " + mode_str + " " + param_str + "\r\n");
			}
		}
		else if (mode_str == "+o" && std::getline(stream, param_str, ' '))
		{
			user_ptr = channel_ptr->second.findUser(param_str);
			if (user_ptr == NULL)
				user.sendMsg("441 " + param_str + " " + channel_str + " :They aren't on that channel\r\n");
			else
			{
				user_ptr->second = true;
				channel_ptr->second.sendToAll(":localhost MODE " + channel_str + " " + mode_str + " " + param_str + "\r\n", &user);
				user.sendMsg(":localhost MODE " + channel_str + " " + mode_str + " " + param_str + "\r\n");
			}
		}
		else
			user.sendMsg("472 " + mode_str + " :is unknown to me\r\n");
		if (std::getline(stream, mode_str, ' ') == NULL)
			break;
	}
	return ("");
}

std::string Server::TopicCmd(std::stringstream &stream, User &user)
{
	std::string str;
	std::map<std::string, Channel>::pointer channel_ptr;

	if (std::getline(stream, str, ' ') == NULL || !(str[0] == '#' || str[0] == '&' || str[0] == '!' || str[0] == '+'))
		return ("461 PART :Not enough parameters\r\n");
	channel_ptr = user.findChannel(str);
	if (channel_ptr == NULL)
		return ("442 " + str + " :You're not on that channel\r\n");
	if (std::getline(stream, str, '\0') == NULL)
	{
		if (channel_ptr->second.getTopic()  == ":")
			return ("331 " + channel_ptr->first + " :No topic is set\r\n");
		else
			return ("332 " + channel_ptr->first + " " + channel_ptr->second.getTopic() + "\r\n");
	}
	else
	{
		if (channel_ptr->second.isUserAllowedToChangeTopic(&user) == false)
			return ("482 " + channel_ptr->first + " :You're not channel operator\r\n");
		channel_ptr->second.setTopic(str);
		channel_ptr->second.sendToAll(":" + user.getNick() + " TOPIC " + channel_ptr->first + " " + channel_ptr->second.getTopic() + "\r\n", &user);
		user.sendMsg(":" + user.getNick() + " TOPIC " + channel_ptr->first + " " + channel_ptr->second.getTopic() + "\r\n");
		return ("");
	}
}

std::string Server::ListCmd(std::stringstream &stream, User &user)
{
	(void) user;
	std::string ret;
	std::string str;
	std::string topic;
	std::string channel;
	std::stringstream	channels_stream;
	std::map<std::string, Channel>::iterator ch_iter;

	std::getline(stream, str, ' ');
	if (str.length() == 0)
	{
		for(ch_iter = channels.begin(); ch_iter != channels.end(); ch_iter++)
		{
			topic = ch_iter->second.getTopic().substr(1);
			if (topic.length() == 0)
				topic = ":";
			std::string count = SSTR(ch_iter->second.getUserCount());
			ret += "322 " + ch_iter->first.substr(1) + " " + "#" + count + " " + topic + "\r\n";
		}
	}
	else
	{
		channels_stream << str;
		while(std::getline(channels_stream, channel, ','))
		{
			ch_iter = channels.find(channel);
			if(ch_iter != channels.end())
			{
				topic = ch_iter->second.getTopic().substr(1);
				if (topic.length() == 0)
					topic = ":";
				std::string count = SSTR(ch_iter->second.getUserCount());
				ret += "322 " + ch_iter->first.substr(1) + " " + "#" + count + " " + topic + "\r\n";
			}
		}
	}
	ret += "323 :End of LIST\r\n";
	return (ret);
}

std::string Server::KickCmd(std::stringstream &stream, User &user)
{
	std::string									channel_str;
	std::string									user_str;
	std::string									ret;
	std::string									reason;
	std::stringstream							channels_stream;
	std::stringstream							users_stream;
	std::stringstream							part_stream;
	std::map<User *, bool>::pointer				user_ptr;
	std::map<std::string, Channel>::iterator	channel_iter;
	std::map<User *, bool>::pointer				ch_user_ptr;

	if ((std::getline(stream, channel_str, ' ') ==  NULL) || (std::getline(stream, user_str, ' ') == NULL))
		return ("461 KICK :Not enough parameters\r\n");
	if (std::getline(stream, reason, '\0') == NULL || reason[0] != ':')
		reason = ":" + user.getNick();
	if (channel_str.find(',') != std::string::npos)
		channels_stream << channel_str;
	users_stream << user_str;
	while (std::getline(users_stream, user_str, ' ') !=  NULL)
	{
		if (channels_stream.str() != "")
			if (std::getline(channels_stream, channel_str, ',') ==  NULL)
				return (ret + "461 KICK :Not enough parameters\r\n");
		channel_iter = channels.find(channel_str);
		if (channel_iter == channels.end())
		{
			ret += "403  " + channel_str + " :No such channel\r\n";
			continue;
		}
		ch_user_ptr = channel_iter->second.findUser(user.getNick());
		if (ch_user_ptr == NULL)
		{
			ret += "442 " + channel_str + " :You're not on that channel\r\n";
			continue;
		}
		if (ch_user_ptr->second == false)
		{
			ret += "482 " + channel_str + " :You're not channel operator\r\n";
			continue;
		}
		user_ptr = channel_iter->second.findUser(user_str);
		if (user_ptr == NULL)
		{
			ret += "441 " + user_str + " " + channel_str + " :They aren't on that channel\r\n";
			continue;
		}
		channel_iter->second.sendToAll(":" + user.getNick() + " KICK " + channel_str + " " + user_str + " " + reason + "\r\n", &user);
		ret += ":" + user.getNick() + " KICK " + channel_str + " " + user_str + " " + reason + "\r\n";
		user_ptr->first->leaveChannel(&(*channel_iter));
		if (channel_iter->second.getUserCount() == 1)
			channels.erase(channel_iter);
		else
			channel_iter->second.removeUser(user_ptr->first);
	}
	return (ret);
}

std::string Server::PrivmsgCmd(std::stringstream &stream, User &user)
{
	std::string									recipent;
	std::string									msg;
	std::string									ret;
	std::map<std::string, Channel>::pointer		channel_ptr;
	std::list<User>::iterator					user_iter;
	std::stringstream							rep_stream;

	if (std::getline(stream, recipent, ' ') == NULL || (std::getline(stream, msg, '\0') == NULL && msg[0] != ':'))
		return ("461 PRIVMSG :Not enough parameters\r\n");
	rep_stream << recipent;
	while (std::getline(rep_stream, recipent, ',') != NULL)
	{
		if (recipent[0] == '#' || recipent[0] == '&' || recipent[0] == '!' || recipent[0] == '+')
		{
			channel_ptr = user.findChannel(recipent);
			if (channel_ptr != NULL)
				channel_ptr->second.sendToAll(":" + user.getNick() + " PRIVMSG " + recipent + " " + msg + "\r\n", &user);
			else
				ret += "401 " + recipent + " :No such channel\r\n";
		}
		else
		{
			user_iter = users.begin();
			while (user_iter != users.end())
			{
				if (user_iter->getNick() == recipent)
				{
					user_iter->sendMsg(":" + user.getNick() + " PRIVMSG " + recipent + " " + msg + "\r\n");
					break;
				}
				++user_iter;
			}
			if (user_iter == users.end())
				ret += "401 " + recipent + " :No such nick\r\n";
		}
	}
	return (ret);
}

std::string Server::NoticeCmd(std::stringstream &stream, User &user)
{
	std::string									recipent;
	std::string									msg;
	std::string									ret;
	std::map<std::string, Channel>::iterator	channel_iter;
	std::list<User>::iterator					user_iter;
	std::stringstream							rep_stream;

	if (std::getline(stream, recipent, ' ') == NULL || (std::getline(stream, msg, '\0') == NULL && msg[0] != ':'))
		return ("461 NOTICE :Not enough parameters\r\n");
	rep_stream << recipent;
	while (std::getline(rep_stream, recipent, ',') != NULL)
	{
		if (recipent[0] == '#' || recipent[0] == '&' || recipent[0] == '!' || recipent[0] == '+')
		{
			channel_iter = channels.find(recipent);
			if (channel_iter != channels.end())
				channel_iter->second.sendToAll(":" + user.getNick() + " NOTICE " + recipent + " " + msg + "\r\n", &user);
			else
				ret += "401 " + recipent + " :No such channel\r\n";
		}
		else
		{
			user_iter = users.begin();
			while (user_iter != users.end())
			{
				if (user_iter->getNick() == recipent)
				{
					user_iter->sendMsg(":" + user.getNick() + " NOTICE " + recipent + " " + msg + "\r\n");
					break;
				}
				++user_iter;
			}
			if (user_iter == users.end())
				ret += "401 " + recipent + " :No such nick\r\n";
		}
	}
	return (ret);
}
