#include "../../inc/Commands.hpp"
#include <map>
#include <set>

bool isValidNick(const std::string& nick)
{
    if (nick.empty() || nick.size() > 9)
        return false;

    static const std::string special = "_-[]\\`^{}|";

    for (std::size_t i = 0; i < nick.size(); ++i)
    {
        char c = nick[i];
        bool isLetter  = (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
        bool isDigit   = (c >= '0' && c <= '9');
        bool isSpecial = special.find(c) != std::string::npos;

        if (i == 0 && !isLetter && !isSpecial)
            return false;
        if (!isLetter && !isDigit && !isSpecial)
            return false;
    }
    return true;
}

void Command::NICK(Client& client, const IrcMessage& msg)
{
    /* RFC 1459 4.1.1: la password va impostata prima di ogni tentativo
       di registrazione, quindi PASS deve precedere NICK/USER */
    if (!client.isPassOk())
        throw NumericError(464, ":Password required");

    if (msg.params.empty())
        throw NumericError(431, ":No nickname given");

    const std::string& newNick = msg.params[0];

    if (!isValidNick(newNick))
        throw NumericError(432, newNick + " :Erroneus nickname");

    std::map<int, Client>& clients = _server.getClients();
    for (std::map<int, Client>::iterator it = clients.begin();
         it != clients.end(); ++it)
    {
        if (it->first != client.getFd() && it->second.getNickname() == newNick)
            throw NumericError(433, newNick + " :Nickname is already in use");
    }

    bool wasRegistered = client.isRegistered();

    if (wasRegistered)
    {
        std::string line = userPrefix(client) + " NICK :" + newNick + "\r\n";
        _server.sendToClient(client.getFd(), line);

        std::set<int> notified;
        std::map<std::string, Channel>& channels = _server.getChannels();
        for (std::map<std::string, Channel>::iterator ch = channels.begin();
             ch != channels.end(); ++ch)
        {
            if (!ch->second.hasClient(&client))
                continue;
            const std::vector<Client*>& members = ch->second.getClients();
            for (std::size_t m = 0; m < members.size(); ++m)
            {
                if (members[m] != &client && notified.insert(members[m]->getFd()).second)
                    _server.sendToClient(members[m]->getFd(), line);
            }
        }
    }
    client.setNickname(newNick);
    client.setNickOk(true);
    if (!wasRegistered && client.isRegistered())
        sendWelcome(client, _server);
}
