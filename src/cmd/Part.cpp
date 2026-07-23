#include "../../inc/Commands.hpp"
#include "../../inc/CommandUtils.hpp"
#include <cstddef>
#include <string>
#include <vector>

Part::Part(Server &server) : ACommand(server) {}

Part::~Part(void) {}

void Part::partOne(Client& client, const std::string& name, const std::string& reason)
{
    Channel* chan = findChannel(_server, name);
    if (!chan)
    {
        numeric(client, 403, name + " :No such channel");
        return;
    }
    if (!chan->hasClient(&client))
    {
        numeric(client, 442, name + " :You're not on that channel");
        return;
    }

    std::string line = userPrefix(client) + " PART " + name;

    if (!reason.empty())
        line += " :" + reason;
    broadcastToChannel(_server, *chan, line + "\r\n", -1);

    chan->removeClient(&client);
    if (chan->isEmpty())
        _server.getChannels().erase(name);
}

void Part::execute(Client& client, const IrcMessage& msg)
{
    if (msg.params.empty())
        throw NumericError(461, "PART :Not enough parameters");

    std::vector<std::string> names = splitComma(msg.params[0]);

    for (std::size_t i = 0; i < names.size(); ++i)
        partOne(client, names[i], msg.trailing);
}
