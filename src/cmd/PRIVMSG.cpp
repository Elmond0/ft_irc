#include "../../inc/PRIVMSG.hpp"
#include "../../inc/CommandUtils.hpp"
#include "../../inc/Bot.hpp"
#include <cstddef>
#include <vector>

PRIVMSG::PRIVMSG(Server &server) : ACommand(server) {}

PRIVMSG::~PRIVMSG(void) {}

void PRIVMSG::sendOne(Client &client, const std::string &target, const std::string &text)
{
	std::string line = userPrefix(client) + " PRIVMSG " + target + " :" + text;

	if (target[0] == '#')
	{
		Channel* chan = findChannel(_server, target);
		if (!chan)
			return numeric(client, 401, target + " :No such nick/channel");
		if (!chan->hasClient(&client))
			return numeric(client, 404, target + " :Cannot send to channel");
		chan->broadcast(line, &client);
		Bot::onMessage(_server, client, chan, text);
		return;
	}

	Client* dest = findClientByNick(_server, target);
	if (!dest)
	{
		if (Bot::isBotNick(target))
			return Bot::onMessage(_server, client, NULL, text);
		return numeric(client, 401, target + " :No such nick/channel");
	}
	dest->queueMessage(line);
}

void PRIVMSG::execute(Client &client, const IrcMessage &msg)
{
	if (msg.params.empty())
		throw NumericError(411, ":No recipient given (PRIVMSG)");
	if (msg.trailing.empty())
		throw NumericError(412, ":No text to send");

	std::vector<std::string> targets = splitComma(msg.params[0]);

	for (std::size_t i = 0; i < targets.size(); ++i)
	{
		if (targets[i].empty())
			continue;
		sendOne(client, targets[i], msg.trailing);
	}
}
