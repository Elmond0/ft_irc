#include "../inc/Commands.hpp"

void NOTICE(Client& client, const IrcMessage& msg, Server& server)
{
    if (msg.params.empty() || msg.trailing.empty())
        return;

    const std::string& target = msg.params[0];
    std::string line = userPrefix(client) + " NOTICE " + target +
        " :" + msg.trailing + "\r\n";

    if (target[0] == '#')
    {
        Channel* chan = findChannel(server, target);
        if (chan && chan->isMember(client.getFd()))
            broadcastToChannel(server, *chan, line, client.getFd());
        return;
    }

    Client* dest = findClientByNick(server, target);
    if (dest)
        server.sendToClient(dest->getFd(), line);
}
