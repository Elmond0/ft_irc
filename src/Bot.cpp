#include "../inc/Bot.hpp"
#include "../inc/CommandUtils.hpp"
#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

#define BOT_NICK	"ircbot"
#define BOT_PREFIX	":" BOT_NICK "!bot@" SERVER_NAME

typedef void	(*BotHandler)(Server &server, Client &sender, Channel *chan, const std::string &args);

struct BotCommand
{
	const char	*name;
	const char	*usage;
	BotHandler	handler;
};

static void	say(Client &sender, Channel *chan, const std::string &text)
{
	std::string line = std::string(BOT_PREFIX) + " PRIVMSG " + (chan ? chan->getName() : sender.getNickname()) + " :" + text;

	if (chan)
		chan->broadcast(line);
	else
		sender.queueMessage(line);
}

static void	cmdHelp(Server &, Client &sender, Channel *chan, const std::string &);
static void	cmdUsers(Server &server, Client &sender, Channel *chan, const std::string &);
static void	cmdTopic(Server &, Client &sender, Channel *chan, const std::string &);

static const BotCommand	g_commands[] = {
	{ "!help",  "!help   list available commands",          cmdHelp },
	{ "!users", "!users  connected users and channel members", cmdUsers },
	{ "!topic", "!topic  show the channel topic",           cmdTopic },
	{ NULL, NULL, NULL }
};

static void	cmdHelp(Server &, Client &sender, Channel *chan, const std::string &)
{
	for (std::size_t i = 0; g_commands[i].name; ++i)
		say(sender, chan, g_commands[i].usage);
}

static void	cmdUsers(Server &server, Client &sender, Channel *chan, const std::string &)
{
	std::ostringstream total;

	total << "users connected to server: " << server.getClients().size();
	say(sender, chan, total.str());

	if (!chan)
		return;

	const std::vector<Client *>& members = chan->getClients();
	std::string names;

	for (std::size_t i = 0; i < members.size(); ++i)
	{
		if (!names.empty())
			names += " ";
		if (chan->isOperator(members[i]))
			names += "@";
		names += members[i]->getNickname();
	}

	std::ostringstream here;
	here << chan->getName() << " (" << members.size() << "): " << names;
	say(sender, chan, here.str());
}

static void	cmdTopic(Server &, Client &sender, Channel *chan, const std::string &)
{
	if (!chan)
	{
		say(sender, chan, "!topic only works inside a channel");
		return;
	}
	if (chan->getTopic().empty())
		say(sender, chan, chan->getName() + " has no topic");
	else
		say(sender, chan, chan->getName() + " topic: " + chan->getTopic());
}

bool Bot::isBotNick(const std::string &nick)
{
	return sameNick(nick, BOT_NICK);
}

void Bot::onMessage(Server &server, Client &sender, Channel *chan, const std::string &text)
{
	std::size_t start = text.find_first_not_of(" \t");

	if (start == std::string::npos)
		return;

	std::string line = text.substr(start);

	if (line[0] != '!')
		return;
	if (isBotNick(sender.getNickname()))
		return;

	std::string name = line;
	std::string args;
	std::size_t space = line.find_first_of(" \t");

	if (space != std::string::npos)
	{
		name = line.substr(0, space);

		std::size_t argStart = line.find_first_not_of(" \t", space);

		if (argStart != std::string::npos)
			args = line.substr(argStart);
	}

	for (std::size_t i = 0; g_commands[i].name; ++i)
	{
		if (!sameNick(name, g_commands[i].name))
			continue;
		g_commands[i].handler(server, sender, chan, args);
		return;
	}

	if (!chan)
		say(sender, chan, "unknown command, try !help");
}
