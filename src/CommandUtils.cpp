#include "../inc/CommandUtils.hpp"
#include <cstddef>
#include <ctime>
#include <map>
#include <string>
#include <vector>

static const std::time_t g_startTime = std::time(NULL);

static const std::string &serverCreationDate(void)
{
	static std::string date;

	if (date.empty())
	{
		char buf[64];
		std::tm *tm = std::localtime(&g_startTime);

		if (tm && std::strftime(buf, sizeof(buf), "%a %b %d %Y at %H:%M:%S %Z", tm))
			date = buf;
		else
			date = "an unknown date";
	}
	return date;
}

static char ircLower(char c)
{
	if (c >= 'A' && c <= 'Z')
		return c + 32;
	switch (c)
	{
		case '[':  return '{';
		case ']':  return '}';
		case '\\': return '|';
	}
	return c;
}

bool sameNick(const std::string &a, const std::string &b)
{
	if (a.size() != b.size())
		return false;
	for (std::size_t i = 0; i < a.size(); ++i)
	{
		if (ircLower(a[i]) != ircLower(b[i]))
			return false;
	}
	return true;
}

std::string nickOrStar(const Client &client)
{
	if (client.getNickname().empty())
		return "*";
	return client.getNickname();
}

std::string userPrefix(const Client &client)
{
	return std::string(":") + client.getPrefix();
}

Client *findClientByNick(Server &server, const std::string &nick)
{
	std::map<int, Client> &clients = server.getClients();

	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		if (sameNick(it->second.getNickname(), nick))
			return &it->second;
	}
	return NULL;
}

Channel *findChannel(Server &server, const std::string &name)
{
	std::map<std::string, Channel> &channels = server.getChannels();
	std::map<std::string, Channel>::iterator it = channels.find(name);

	if (it == channels.end())
		return NULL;
	return &it->second;
}

std::vector<std::string> splitComma(const std::string &s)
{
	std::vector<std::string> out;
	std::size_t start = 0;

	while (start <= s.size())
	{
		std::size_t comma = s.find(',', start);
		if (comma == std::string::npos)
		{
			out.push_back(s.substr(start));
			break;
		}
		out.push_back(s.substr(start, comma - start));
		start = comma + 1;
	}
	return out;
}

void finishRegistrationAttempt(Client &client)
{
	if (!client.isRegistered())
		return;
	sendWelcome(client);
}

void sendWelcome(Client &client)
{
	std::string nick = client.getNickname();

	client.queueMessage(std::string(":") + SERVER_NAME + " 001 " + nick + " :Welcome to the " + NETWORK_NAME + " IRC Network " + client.getPrefix());
	client.queueMessage(std::string(":") + SERVER_NAME + " 002 " + nick + " :Your host is " + SERVER_NAME + ", running version " + SERVER_VERSION);
	client.queueMessage(std::string(":") + SERVER_NAME + " 003 " + nick + " :This server was created " + serverCreationDate());
	client.queueMessage(std::string(":") + SERVER_NAME + " 004 " + nick + " " + SERVER_NAME + " " + SERVER_VERSION + " " + USER_MODES + " " + CHANNEL_MODES);
}