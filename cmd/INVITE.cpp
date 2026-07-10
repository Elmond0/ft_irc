#include "../inc/Commands.hpp"

void INVITE(Client& client, const IrcMessage& msg, Server& server)
{
    if (msg.params.size() < 2)
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 461 " + client.getNickname() + " INVITE :Not enough parameters\r\n");
        return;
    }

    const std::string& targetNick = msg.params[0];
    const std::string& chanName = msg.params[1];

    Client* target = findClientByNick(server, targetNick);
    if (!target)
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 401 " + client.getNickname() + " " + targetNick + " :No such nick/channel\r\n");
        return;
    }

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
    if (chan->isInviteOnly() && !chan->isOperator(&client))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 482 " + client.getNickname() + " " + chanName + " :You're not channel operator\r\n");
        return;
    }
    if (chan->hasClient(target))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 443 " + client.getNickname() + " " + targetNick + " " + chanName +
            " :is already on channel\r\n");
        return;
    }

    chan->addInvited(target);
    server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
        " 341 " + client.getNickname() + " " + targetNick + " " + chanName + "\r\n");
    server.sendToClient(target->getFd(),
        userPrefix(client) + " INVITE " + targetNick + " :" + chanName + "\r\n");
}
