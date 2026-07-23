#include "../../inc/Mode.hpp"
#include "../../inc/CommandUtils.hpp"
#include <cstddef>
#include <cstdlib>
#include <sstream>
#include <string>

Mode::Mode(Server &server) : ACommand(server) {}

Mode::~Mode(void) {}

void Mode::sendChannelModes(Client& client, Channel& chan)
{
    std::string modes = chan.getModeString();

    if (modes.empty())
        modes = "+";
    std::string rest = chan.getName() + " " + modes;
    if (!chan.getKey().empty())
        rest += " " + chan.getKey();
    if (chan.getUserLimit() > 0)
    {
        std::ostringstream oss;
        oss << chan.getUserLimit();
        rest += " " + oss.str();
    }
    numeric(client, 324, rest);
}

bool Mode::nextArg(const IrcMessage& msg, std::size_t& idx, std::string& out)
{
    if (idx >= msg.params.size())
        return false;
    out = msg.params[idx++];
    return true;
}

static bool parseLimit(const std::string& str, long& out)
{
    if (str.empty())
        return false;
    char* end = 0;
    out = std::strtol(str.c_str(), &end, 10);
    return *end == '\0' && out > 0;
}

bool Mode::applyOneMode(Client& client, Channel& chan, char c, bool adding, const IrcMessage& msg, std::size_t& argIdx, std::string& usedArg)
{
    std::string arg;

    switch (c)
    {
        case 'i':
            chan.setInviteOnly(adding);
            return true;

        case 't':
            chan.setTopicRestricted(adding);
            return true;

        case 'k':
            if (!adding)
            {
                chan.clearKey();
                nextArg(msg, argIdx, arg);
                usedArg = "*";
                return true;
            }
            if (nextArg(msg, argIdx, arg))
            {
                if (chan.hasKey())
                {
                    numeric(client, 467, chan.getName() +
                        " :Channel key already set");
                    return false;
                }
                chan.setKey(arg);
                usedArg = arg;
                return true;
            }
            numeric(client, 461, "MODE :Not enough parameters");
            return false;

        case 'o':
        {
            if (!nextArg(msg, argIdx, arg))
            {
                numeric(client, 461, "MODE :Not enough parameters");
                return false;
            }
            Client* member = findClientByNick(_server, arg);
            if (!member || !chan.hasClient(member))
            {
                numeric(client, 441, arg + " " + chan.getName() + " :They aren't on that channel");
                return false;
            }
            if (adding)
                chan.addOperator(member);
            else
                chan.removeOperator(member);
            usedArg = arg;
            return true;
        }

        case 'l':
        {
            long limit = 0;
            if (!adding)
            {
                chan.clearUserLimit();
                return true;
            }
            if (nextArg(msg, argIdx, arg) && parseLimit(arg, limit))
            {
                chan.setUserLimit(static_cast<std::size_t>(limit));
                usedArg = arg;
                return true;
            }
            numeric(client, 461, "MODE :Not enough parameters");
            return false;
        }

        default:
            numeric(client, 472, std::string(1, c) + " :is unknown mode char to me");
            return false;
    }
}

void Mode::applyModes(Client& client, Channel& chan, const IrcMessage& msg)
{
    const std::string& modes = msg.params[1];
    std::size_t argIdx = 2;
    bool adding = true;

    std::string appliedModes;
    std::string appliedArgs;
    char lastSign = 0;

    for (std::size_t i = 0; i < modes.size(); ++i)
    {
        char c = modes[i];

        if (c == '+') { adding = true; continue; }
        if (c == '-') { adding = false; continue; }

        std::string usedArg;
        if (!applyOneMode(client, chan, c, adding, msg, argIdx, usedArg))
            continue;

        char sign = adding ? '+' : '-';
        if (sign != lastSign)
        {
            appliedModes += sign;
            lastSign = sign;
        }
        appliedModes += c;
        if (!usedArg.empty())
            appliedArgs += " " + usedArg;
    }

    if (!appliedModes.empty())
        broadcastToChannel(_server, chan, userPrefix(client) + " MODE " + chan.getName() + " " + appliedModes + appliedArgs + "\r\n", -1);
}

void Mode::execute(Client& client, const IrcMessage& msg)
{
    if (msg.params.empty())
        throw NumericError(461, "MODE :Not enough parameters");

    const std::string& target = msg.params[0];

    if (target[0] != '#')
    {
        Client* targetUser = findClientByNick(_server, target);
        if (!targetUser)
            throw NumericError(401, target + " :No such nick/channel");
        if (targetUser != &client)
            throw NumericError(502, ":Cannot change mode for other users");
        numeric(client, 221, "+"); /* RPL_UMODEIS: nessun user mode supportato */
        return;
    }

    Channel* chan = findChannel(_server, target);

    if (!chan)
        throw NumericError(403, target + " :No such channel");
    if (!chan->hasClient(&client))
        throw NumericError(442, target + " :You're not on that channel");

    if (msg.params.size() == 1)
    {
        sendChannelModes(client, *chan);
        return;
    }

    if (!chan->isOperator(&client))
        throw NumericError(482, target + " :You're not channel operator");

    applyModes(client, *chan, msg);
}
