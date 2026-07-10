#include "../inc/Commands.hpp"

void KICK(Client& client, const IrcMessage& msg, Server& server)
{
    if (msg.params.size() < 2)
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 461 " + client.getNickname() + " KICK :Not enough parameters\r\n");
        return;
    }

    const std::string& chanName = msg.params[0];
    const std::string& targetNick = msg.params[1];

    Channel* chan = findChannel(server, chanName);
    if (!chan)
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 403 " + client.getNickname() + " " + chanName + " :No such channel\r\n");
        return;
    }
    if (!chan->hasClient(&client))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 442 " + client.getNickname() + " " + chanName + " :You're not on that channel\r\n");
        return;
    }
    if (!chan->isOperator(&client))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 482 " + client.getNickname() + " " + chanName + " :You're not channel operator\r\n");
        return;
    }

    Client* target = findClientByNick(server, targetNick);
    if (!target || !chan->hasClient(target))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 441 " + client.getNickname() + " " + targetNick + " " + chanName +
            " :They aren't on that channel\r\n");
        return;
    }

    std::string reason = msg.trailing.empty() ? client.getNickname() : msg.trailing;

    broadcastToChannel(server, *chan,
        userPrefix(client) + " KICK " + chanName + " " + targetNick +
        " :" + reason + "\r\n", -1);
    chan->removeClient(target);
    if (chan->isEmpty())
        server.getChannels().erase(chanName);
}
