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
    const std::vector<Client*>& members = chan.getClients();

    for (std::size_t i = 0; i < members.size(); ++i)
    {
        if (!names.empty())
            names += " ";
        if (chan.isOperator(members[i]))
            names += "@";
        names += members[i]->getNickname();
    }

    server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
        " 353 " + client.getNickname() + " = " + chan.getName() + " :" + names + "\r\n");
    server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
        " 366 " + client.getNickname() + " " + chan.getName() + " :End of /NAMES list\r\n");
}

static void joinOne(Client& client, Server& server,
                    const std::string& name, const std::string& key)
{
    if (!isValidChannelName(name))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 403 " + client.getNickname() + " " + name + " :No such channel\r\n");
        return;
    }

    std::map<std::string, Channel>& channels = server.getChannels();
    bool isNew = channels.find(name) == channels.end();
    if (isNew)
        channels.insert(std::make_pair(name, Channel(name)));
    Channel& chan = channels[name];

    if (chan.hasClient(&client))
        return;

    if (chan.isInviteOnly() && !chan.isInvited(&client))
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 473 " + client.getNickname() + " " + name + " :Cannot join channel (+i)\r\n");
        return;
    }
    if (chan.hasKey() && key != chan.getKey())
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 475 " + client.getNickname() + " " + name + " :Cannot join channel (+k)\r\n");
        return;
    }
    if (chan.getUserLimit() > 0 && chan.getClients().size() >= chan.getUserLimit())
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 471 " + client.getNickname() + " " + name + " :Cannot join channel (+l)\r\n");
        return;
    }

    chan.addClient(&client);
    chan.removeInvited(&client); /* l'invito si consuma al join */
    if (isNew)
        chan.addOperator(&client); /* il creatore diventa op */

    broadcastToChannel(server, chan,
        userPrefix(client) + " JOIN " + name + "\r\n", -1);

    if (chan.getTopic().empty())
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 331 " + client.getNickname() + " " + name + " :No topic is set\r\n");
    else
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 332 " + client.getNickname() + " " + name + " :" + chan.getTopic() + "\r\n");

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
        if (!chan.hasClient(&client))
            continue;
        broadcastToChannel(server, chan,
            userPrefix(client) + " PART " + chan.getName() + "\r\n", -1);
        chan.removeClient(&client);
        if (chan.isEmpty())
            channels.erase(current);
    }
}

void JOIN(Client& client, const IrcMessage& msg, Server& server)
{
    if (msg.params.empty())
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 461 " + client.getNickname() + " JOIN :Not enough parameters\r\n");
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
