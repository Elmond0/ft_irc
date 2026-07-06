#include "../inc/Commands.hpp"

void handle_KICK(Client& client, const IrcMessage& msg, Server& server)
{
    if (msg.params.size() < 2)
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 461 " + client.getNick() + " KICK :Not enough parameters\r\n");
        return;
    }

    const std::string& chanName = msg.params[0];
    const std::string& targetNick = msg.params[1];

    Channel* chan = findChannel(server, chanName);
    if (!chan)
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 403 " + client.getNick() + " " + chanName + " :No such channel\r\n");
        return;
    }
    if (!chan->isMember(client.getFd()))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 442 " + client.getNick() + " " + chanName + " :You're not on that channel\r\n");
        return;
    }
    if (!chan->isOperator(client.getFd()))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 482 " + client.getNick() + " " + chanName + " :You're not channel operator\r\n");
        return;
    }

    Client* target = findClientByNick(server, targetNick);
    if (!target || !chan->isMember(target->getFd()))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 441 " + client.getNick() + " " + targetNick + " " + chanName +
            " :They aren't on that channel\r\n");
        return;
    }

    std::string reason = msg.trailing.empty() ? client.getNick() : msg.trailing;

    broadcastToChannel(server, *chan,
        userPrefix(client) + " KICK " + chanName + " " + targetNick +
        " :" + reason + "\r\n", -1);
    chan->removeMember(target->getFd());
    if (chan->isEmpty())
        server.getChannels().erase(chanName);
}
