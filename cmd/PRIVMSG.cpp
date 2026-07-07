#include "../inc/Commands.hpp"

void handle_PRIVMSG(Client& client, const IrcMessage& msg, Server& server)
{
    if (msg.params.empty())
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 411 " + client.getNick() + " :No recipient given (PRIVMSG)\r\n");
        return;
    }
    if (msg.trailing.empty())
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 412 " + client.getNick() + " :No text to send\r\n");
        return;
    }

    const std::string& target = msg.params[0];
    std::string line = userPrefix(client) + " PRIVMSG " + target +
        " :" + msg.trailing + "\r\n";

    if (target[0] == '#')
    {
        Channel* chan = findChannel(server, target);
        if (!chan)
        {
            server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
                " 401 " + client.getNick() + " " + target + " :No such nick/channel\r\n");
            return;
        }
        if (!chan->isMember(client.getFd()))
        {
            server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
                " 404 " + client.getNick() + " " + target + " :Cannot send to channel\r\n");
            return;
        }
        broadcastToChannel(server, *chan, line, client.getFd());
        return;
    }

    Client* dest = findClientByNick(server, target);
    if (!dest)
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 401 " + client.getNick() + " " + target + " :No such nick/channel\r\n");
        return;
    }
    server.sendToClient(dest->getFd(), line);
}
