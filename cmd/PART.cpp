#include "../inc/Commands.hpp"

static void partOne(Client& client, Server& server,
                    const std::string& name, const std::string& reason)
{
    Channel* chan = findChannel(server, name);
    if (!chan)
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 403 " + client.getNick() + " " + name + " :No such channel\r\n");
        return;
    }
    if (!chan->isMember(client.getFd()))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 442 " + client.getNick() + " " + name + " :You're not on that channel\r\n");
        return;
    }

    std::string line = userPrefix(client) + " PART " + name;
    if (!reason.empty())
        line += " :" + reason;
    broadcastToChannel(server, *chan, line + "\r\n", -1);

    chan->removeMember(client.getFd());
    if (chan->isEmpty())
        server.getChannels().erase(name);
}

void handle_PART(Client& client, const IrcMessage& msg, Server& server)
{
    if (msg.params.empty())
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 461 " + client.getNick() + " PART :Not enough parameters\r\n");
        return;
    }

    std::vector<std::string> names = splitComma(msg.params[0]);
    for (std::size_t i = 0; i < names.size(); ++i)
        partOne(client, server, names[i], msg.trailing);
}
