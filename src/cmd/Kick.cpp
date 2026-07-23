#include "../../inc/Commands.hpp"
#include "../../inc/CommandUtils.hpp"

Kick::Kick(Server &server) : ACommand(server) {}

Kick::~Kick(void) {}

void Kick::execute(Client &client, const IrcMessage &msg)
{
    if (msg.params.size() < 2)
        throw NumericError(461, "KICK :Not enough parameters");

    const std::string& chanName = msg.params[0];
    const std::string& targetNick = msg.params[1];

    Channel* chan = findChannel(_server, chanName);
    if (!chan)
        throw NumericError(403, chanName + " :No such channel");
    if (!chan->hasClient(&client))
        throw NumericError(442, chanName + " :You're not on that channel");
    if (!chan->isOperator(&client))
        throw NumericError(482, chanName + " :You're not channel operator");

    Client* target = findClientByNick(_server, targetNick);
    if (!target || !chan->hasClient(target))
        throw NumericError(441, targetNick + " " + chanName +
            " :They aren't on that channel");

    std::string reason = msg.trailing.empty() ? client.getNickname() : msg.trailing;

    broadcastToChannel(_server, *chan,
        userPrefix(client) + " KICK " + chanName + " " + targetNick +
        " :" + reason + "\r\n", -1);
    chan->removeClient(target);
    if (chan->isEmpty())
        _server.getChannels().erase(chanName);
}
