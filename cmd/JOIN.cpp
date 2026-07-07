#include "../inc/Commands.hpp"

static bool isValidChannelName(const std::string& name)
{
    if (name.size() < 2 || name[0] != '#')
        return false;
    return name.find(' ') == std::string::npos &&
           name.find(',') == std::string::npos;
}

static void sendNames(Client& client, Channel& chan, Server& server)
{
    std::string names;
    const std::set<int>& members = chan.getMembers();
    std::map<int, Client>& clients = server.getClients();

    for (std::set<int>::const_iterator it = members.begin();
         it != members.end(); ++it)
    {
        std::map<int, Client>::const_iterator c = clients.find(*it);
        if (c == clients.end())
            continue;
        if (!names.empty())
            names += " ";
        if (chan.isOperator(*it))
            names += "@";
        names += c->second.getNick();
    }

    server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
        " 353 " + client.getNick() + " = " + chan.getName() + " :" + names + "\r\n");
    server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
        " 366 " + client.getNick() + " " + chan.getName() + " :End of /NAMES list\r\n");
}

static void joinOne(Client& client, Server& server,
                    const std::string& name, const std::string& key)
{
    if (!isValidChannelName(name))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 403 " + client.getNick() + " " + name + " :No such channel\r\n");
        return;
    }

    std::map<std::string, Channel>& channels = server.getChannels();
    bool isNew = channels.find(name) == channels.end();
    if (isNew)
        channels.insert(std::make_pair(name, Channel(name)));
    Channel& chan = channels[name];

    if (chan.isMember(client.getFd()))
        return;

    if (chan.isInviteOnly() && !chan.isInvited(client.getFd()))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 473 " + client.getNick() + " " + name + " :Cannot join channel (+i)\r\n");
        return;
    }
    if (!chan.getKey().empty() && key != chan.getKey())
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 475 " + client.getNick() + " " + name + " :Cannot join channel (+k)\r\n");
        return;
    }
    if (chan.isFull())
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 471 " + client.getNick() + " " + name + " :Cannot join channel (+l)\r\n");
        return;
    }

    chan.addMember(client.getFd());
    if (isNew)
        chan.addOperator(client.getFd()); /* il creatore diventa op */

    broadcastToChannel(server, chan,
        userPrefix(client) + " JOIN " + name + "\r\n", -1);

    if (chan.getTopic().empty())
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 331 " + client.getNick() + " " + name + " :No topic is set\r\n");
    else
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 332 " + client.getNick() + " " + name + " :" + chan.getTopic() + "\r\n");

    sendNames(client, chan, server);
}

static void partAll(Client& client, Server& server)
{
    std::map<std::string, Channel>& channels = server.getChannels();
    std::map<std::string, Channel>::iterator it = channels.begin();

    while (it != channels.end())
    {
        std::map<std::string, Channel>::iterator current = it++;
        Channel& chan = current->second;
        if (!chan.isMember(client.getFd()))
            continue;
        broadcastToChannel(server, chan,
            userPrefix(client) + " PART " + chan.getName() + "\r\n", -1);
        chan.removeMember(client.getFd());
        if (chan.isEmpty())
            channels.erase(current);
    }
}

void handle_JOIN(Client& client, const IrcMessage& msg, Server& server)
{
    if (msg.params.empty())
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 461 " + client.getNick() + " JOIN :Not enough parameters\r\n");
        return;
    }

    if (msg.params[0] == "0")
    {
        partAll(client, server);
        return;
    }

    std::vector<std::string> names = splitComma(msg.params[0]);
    std::vector<std::string> keys;
    if (msg.params.size() > 1)
        keys = splitComma(msg.params[1]);

    for (std::size_t i = 0; i < names.size(); ++i)
        joinOne(client, server, names[i], i < keys.size() ? keys[i] : "");
}
