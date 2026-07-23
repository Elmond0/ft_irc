#include "../../inc/TOPIC.hpp"
#include "../../inc/CommandUtils.hpp"

TOPIC::TOPIC(Server &server) : ACommand(server) {}

TOPIC::~TOPIC(void) {}

void TOPIC::execute(Client &client, const IrcMessage &msg)
{
    if (msg.params.empty())
        throw NumericError(461, "TOPIC :Not enough parameters");

    const std::string& chanName = msg.params[0];

    Channel* chan = findChannel(_server, chanName);
    if (!chan)
        throw NumericError(403, chanName + " :No such channel");
    if (!chan->hasClient(&client))
        throw NumericError(442, chanName + " :You're not on that channel");

    if (msg.trailing.empty() && msg.params.size() < 2)
    {
        if (chan->getTopic().empty())
            numeric(client, 331, chanName + " :No topic is set");
        else
            numeric(client, 332, chanName + " :" + chan->getTopic());
        return;
    }

    if (chan->isTopicRestricted() && !chan->isOperator(&client))
        throw NumericError(482, chanName + " :You're not channel operator");

    std::string newTopic = msg.trailing.empty() ? msg.params[1] : msg.trailing;
    chan->setTopic(newTopic);
    broadcastToChannel(_server, *chan,
        userPrefix(client) + " TOPIC " + chanName + " :" + newTopic + "\r\n", -1);
}
