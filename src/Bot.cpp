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
	{ "!help",  "!help   elenca i comandi disponibili",       cmdHelp },
	{ "!users", "!users  utenti connessi e membri del canale", cmdUsers },
	{ "!topic", "!topic  mostra il topic del canale",          cmdTopic },
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

	total << "utenti connessi al server: " << server.getClients().size();
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
		say(sender, chan, "!topic funziona solo dentro un canale");
		return;
	}
	if (chan->getTopic().empty())
		say(sender, chan, chan->getName() + " non ha un topic");
	else
		say(sender, chan, chan->getName() + " topic: " + chan->getTopic());
}

bool Bot::isBotNick(const std::string &nick)
{
	return sameNick(nick, BOT_NICK);
}

void Bot::onMessage(Server &server, Client &sender, Channel *chan, const std::string &text)
{
	if (text.empty() || text[0] != '!')
		return;
	if (isBotNick(sender.getNickname()))
		return;

	std::string name = text;
	std::string args;
	std::size_t space = text.find(' ');

	if (space != std::string::npos)
	{
		name = text.substr(0, space);
		args = text.substr(space + 1);
	}

	for (std::size_t i = 0; g_commands[i].name; ++i)
	{
		if (!sameNick(name, g_commands[i].name))
			continue;
		g_commands[i].handler(server, sender, chan, args);
		return;
	}

	if (!chan)
		say(sender, chan, "comando sconosciuto, prova !help");
}
