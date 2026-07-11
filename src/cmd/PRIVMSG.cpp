#include "../../inc/Commands.hpp"

void Command::PRIVMSG(Client& client, const IrcMessage& msg)
{
    if (msg.params.empty())
        throw NumericError(411, ":No recipient given (PRIVMSG)");
    if (msg.trailing.empty())
        throw NumericError(412, ":No text to send");

    const std::string& target = msg.params[0];
    std::string line = userPrefix(client) + " PRIVMSG " + target +
        " :" + msg.trailing + "\r\n";

    if (target[0] == '#')
    {
        Channel* chan = findChannel(_server, target);
        if (!chan)
            throw NumericError(401, target + " :No such nick/channel");
        if (!chan->hasClient(&client))
            throw NumericError(404, target + " :Cannot send to channel");
        broadcastToChannel(_server, *chan, line, client.getFd());
        return;
    }

    Client* dest = findClientByNick(_server, target);
    if (!dest)
        throw NumericError(401, target + " :No such nick/channel");
    _server.sendToClient(dest->getFd(), line);
}
