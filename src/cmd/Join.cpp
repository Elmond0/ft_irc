#include "../../inc/Commands.hpp"
#include "../../inc/CommandUtils.hpp"
#include <cstddef>
#include <map>
#include <string>
#include <utility>
#include <vector>

Join::Join(Server &server) : ACommand(server) {}

Join::~Join(void) {}

bool Join::isValidChannelName(const std::string& name) const
{
    if (name.size() < 2 || name[0] != '#')
        return false;
    return name.find(' ') == std::string::npos && name.find(',') == std::string::npos;
}

void Join::sendNames(Client& client, Channel& chan)
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

    numeric(client, 353, "= " + chan.getName() + " :" + names);
    numeric(client, 366, chan.getName() + " :End of /NAMES list");
}

void Join::joinOne(Client& client, const std::string& name,
                      const std::string& key)
{
    if (!isValidChannelName(name))
    {
        numeric(client, 403, name + " :No such channel");
        return;
    }

    std::map<std::string, Channel>& channels = _server.getChannels();
    bool isNew = channels.find(name) == channels.end();
    if (isNew)
        channels.insert(std::make_pair(name, Channel(name)));
    Channel& chan = channels[name];

    if (chan.hasClient(&client))
        return;

    if (chan.isInviteOnly() && !chan.isInvited(&client))
    {
        numeric(client, 473, name + " :Cannot join channel (+i)");
        return;
    }
    if (chan.hasKey() && key != chan.getKey())
    {
        numeric(client, 475, name + " :Cannot join channel (+k)");
        return;
    }
    if (chan.getUserLimit() > 0 && chan.getClients().size() >= chan.getUserLimit())
    {
        numeric(client, 471, name + " :Cannot join channel (+l)");
        return;
    }

    chan.addClient(&client);
    chan.removeInvited(&client);
    if (isNew)
        chan.addOperator(&client);

    broadcastToChannel(_server, chan,
        userPrefix(client) + " JOIN " + name + "\r\n", -1);

    if (chan.getTopic().empty())
        numeric(client, 331, name + " :No topic is set");
    else
        numeric(client, 332, name + " :" + chan.getTopic());

    sendNames(client, chan);
}

void Join::partAll(Client& client)
{
    std::map<std::string, Channel>& channels = _server.getChannels();
    std::map<std::string, Channel>::iterator it = channels.begin();

    while (it != channels.end())
    {
        std::map<std::string, Channel>::iterator current = it++;
        Channel& chan = current->second;
        if (!chan.hasClient(&client))
            continue;
        broadcastToChannel(_server, chan,
            userPrefix(client) + " PART " + chan.getName() + "\r\n", -1);
        chan.removeClient(&client);
        if (chan.isEmpty())
            channels.erase(current);
    }
}

void Join::execute(Client& client, const IrcMessage& msg)
{
    if (msg.params.empty())
        throw NumericError(461, "JOIN :Not enough parameters");

    if (msg.params[0] == "0")
    {
        partAll(client);
        return;
    }

    std::vector<std::string> names = splitComma(msg.params[0]);
    std::vector<std::string> keys;
    if (msg.params.size() > 1)
        keys = splitComma(msg.params[1]);

    for (std::size_t i = 0; i < names.size(); ++i)
        joinOne(client, names[i], i < keys.size() ? keys[i] : "");
}
