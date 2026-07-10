#include "../inc/Commands.hpp"

void TOPIC(Client& client, const IrcMessage& msg, Server& server)
{
    if (msg.params.empty())
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 461 " + client.getNickname() + " TOPIC :Not enough parameters\r\n");
        return;
    }

    const std::string& chanName = msg.params[0];

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

    if (msg.trailing.empty() && msg.params.size() < 2)
    {
        if (chan->getTopic().empty())
            server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
                " 331 " + client.getNickname() + " " + chanName + " :No topic is set\r\n");
        else
            server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
                " 332 " + client.getNickname() + " " + chanName + " :" + chan->getTopic() + "\r\n");
        return;
    }

    if (chan->isTopicRestricted() && !chan->isOperator(&client))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 482 " + client.getNickname() + " " + chanName + " :You're not channel operator\r\n");
        return;
    }

    std::string newTopic = msg.trailing.empty() ? msg.params[1] : msg.trailing;
    chan->setTopic(newTopic);
    broadcastToChannel(server, *chan,
        userPrefix(client) + " TOPIC " + chanName + " :" + newTopic + "\r\n", -1);
}
