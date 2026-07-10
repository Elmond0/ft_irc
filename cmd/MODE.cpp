#include "../inc/Commands.hpp"
#include <cstdlib>
#include <sstream>

static void sendChannelModes(Client& client, Channel& chan, Server& server)
{
    std::string reply = std::string(":") + SERVER_NAME + " 324 " +
        client.getNick() + " " + chan.getName() + " " + chan.modeString();
    if (!chan.getKey().empty())
        reply += " " + chan.getKey();
    if (chan.getUserLimit() > 0)
    {
        std::ostringstream oss;
        oss << chan.getUserLimit();
        reply += " " + oss.str();
    }
    server.sendToClient(client.getFd(), reply + "\r\n");
}

static bool nextArg(const IrcMessage& msg, std::size_t& idx, std::string& out)
{
    if (idx >= msg.params.size())
        return false;
    out = msg.params[idx++];
    return true;
}

void MODE(Client& client, const IrcMessage& msg, Server& server)
{
    if (msg.params.empty())
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 461 " + client.getNick() + " MODE :Not enough parameters\r\n");
        return;
    }

    const std::string& target = msg.params[0];
    if (target[0] != '#')
        return;

    Channel* chan = findChannel(server, target);
    if (!chan)
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 403 " + client.getNick() + " " + target + " :No such channel\r\n");
        return;
    }

    if (msg.params.size() == 1)
    {
        sendChannelModes(client, *chan, server);
        return;
    }

    if (!chan->isMember(client.getFd()))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 442 " + client.getNick() + " " + target + " :You're not on that channel\r\n");
        return;
    }
    if (!chan->isOperator(client.getFd()))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 482 " + client.getNick() + " " + target + " :You're not channel operator\r\n");
        return;
    }

    const std::string& modes = msg.params[1];
    std::size_t argIdx = 2;
    bool adding = true;
    std::string arg;

    for (std::size_t i = 0; i < modes.size(); ++i)
    {
        char c = modes[i];

        if (c == '+') { adding = true; continue; }
        if (c == '-') { adding = false; continue; }

        if (c == 'i')
            chan->setInviteOnly(adding);
        else if (c == 't')
            chan->setTopicLocked(adding);
        else if (c == 'k')
        {
            if (!adding)
            {
                chan->setKey("");
                nextArg(msg, argIdx, arg);
            }
            else if (nextArg(msg, argIdx, arg))
                chan->setKey(arg);
            else
                server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
                    " 461 " + client.getNick() + " MODE :Not enough parameters\r\n");
        }
        else if (c == 'o')
        {
            if (!nextArg(msg, argIdx, arg))
            {
                server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
                    " 461 " + client.getNick() + " MODE :Not enough parameters\r\n");
                continue;
            }
            Client* member = findClientByNick(server, arg);
            if (!member || !chan->isMember(member->getFd()))
            {
                server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
                    " 441 " + client.getNick() + " " + arg + " " + target +
                    " :They aren't on that channel\r\n");
                continue;
            }
            if (adding)
                chan->addOperator(member->getFd());
            else
                chan->removeOperator(member->getFd());
        }
        else if (c == 'l')
        {
            if (!adding)
                chan->setUserLimit(0);
            else if (nextArg(msg, argIdx, arg) && std::atoi(arg.c_str()) > 0)
                chan->setUserLimit(std::atoi(arg.c_str()));
            else
                server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
                    " 461 " + client.getNick() + " MODE :Not enough parameters\r\n");
        }
        else
        {
            server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
                " 472 " + client.getNick() + " " + std::string(1, c) +
                " :is unknown mode char to me\r\n");
        }
    }

    std::string applied = modes;
    for (std::size_t i = 2; i < msg.params.size(); ++i)
        applied += " " + msg.params[i];
    broadcastToChannel(server, *chan,
        userPrefix(client) + " MODE " + target + " " + applied + "\r\n", -1);
}
