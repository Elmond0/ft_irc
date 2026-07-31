#include "../../inc/PRIVMSG.hpp"
#include "../../inc/CommandUtils.hpp"
#include "../../inc/Bot.hpp"

PRIVMSG::PRIVMSG(Server &server) : ACommand(server) {}

PRIVMSG::~PRIVMSG(void) {}

void PRIVMSG::execute(Client &client, const IrcMessage &msg)
{
	if (msg.params.empty())
		throw NumericError(411, ":No recipient given (PRIVMSG)");
	if (msg.trailing.empty())
		throw NumericError(412, ":No text to send");

	const std::string& target = msg.params[0];
	std::string line = userPrefix(client) + " PRIVMSG " + target + " :" + msg.trailing;

	if (target[0] == '#')
	{
		Channel* chan = findChannel(_server, target);
		if (!chan)
			throw NumericError(401, target + " :No such nick/channel");
		if (!chan->hasClient(&client))
			throw NumericError(404, target + " :Cannot send to channel");
		chan->broadcast(line, &client);
		Bot::onMessage(_server, client, chan, msg.trailing);
		return;
	}

	Client* dest = findClientByNick(_server, target);
	if (!dest)
	{
		if (Bot::isBotNick(target))
			return Bot::onMessage(_server, client, NULL, msg.trailing);
		throw NumericError(401, target + " :No such nick/channel");
	}
	dest->queueMessage(line);
}