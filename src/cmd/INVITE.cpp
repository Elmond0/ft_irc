#include "../../inc/Commands.hpp"

void Command::INVITE(Client& client, const IrcMessage& msg)
{
    if (msg.params.size() < 2)
        throw NumericError(461, "INVITE :Not enough parameters");

    const std::string& targetNick = msg.params[0];
    const std::string& chanName = msg.params[1];

    Client* target = findClientByNick(_server, targetNick);
    if (!target)
        throw NumericError(401, targetNick + " :No such nick/channel");

    Channel* chan = findChannel(_server, chanName);
    if (!chan)
        throw NumericError(403, chanName + " :No such channel");
    if (!chan->hasClient(&client))
        throw NumericError(442, chanName + " :You're not on that channel");
    if (chan->isInviteOnly() && !chan->isOperator(&client))
        throw NumericError(482, chanName + " :You're not channel operator");
    if (chan->hasClient(target))
        throw NumericError(443, targetNick + " " + chanName +
            " :is already on channel");

    chan->addInvited(target);
    numeric(client, 341, targetNick + " " + chanName);
    _server.sendToClient(target->getFd(), userPrefix(client) + " INVITE " + targetNick + " :" + chanName + "\r\n");
}
