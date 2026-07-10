#include "../inc/Commands.hpp"
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

void NICK(Client& client, const IrcMessage& msg, Server& server)
{
    if (msg.params.empty())
    {
        std::string reply = std::string(":") + SERVER_NAME + " 431 " +
            nickOrStar(client) + " :No nickname given\r\n";
        server.sendToClient(client.getFd(), reply);
        return;
    }

    const std::string& newNick = msg.params[0];

    if (!isValidNick(newNick))
    {
        std::string reply = std::string(":") + SERVER_NAME + " 432 " +
            nickOrStar(client) + " " + newNick + " :Erroneus nickname\r\n";
        server.sendToClient(client.getFd(), reply);
        return;
    }

    std::map<int, Client>& clients = server.getClients();
    for (std::map<int, Client>::iterator it = clients.begin();
         it != clients.end(); ++it)
    {
        if (it->first != client.getFd() && it->second.getNickname() == newNick)
        {
            std::string reply = std::string(":") + SERVER_NAME + " 433 " +
                nickOrStar(client) + " " + newNick +
                " :Nickname is already in use\r\n";
            server.sendToClient(client.getFd(), reply);
            return;
        }
    }

    bool wasRegistered = client.isRegistered();

    /* RFC 1459: il cambio nick va notificato al client stesso e a tutti
       i canali in cui si trova, con il prefisso del VECCHIO nick */
    if (wasRegistered)
    {
        std::string line = userPrefix(client) + " NICK :" + newNick + "\r\n";
        server.sendToClient(client.getFd(), line);

        std::set<int> notified;
        std::map<std::string, Channel>& channels = server.getChannels();
        for (std::map<std::string, Channel>::iterator ch = channels.begin();
             ch != channels.end(); ++ch)
        {
            if (!ch->second.hasClient(&client))
                continue;
            const std::vector<Client*>& members = ch->second.getClients();
            for (std::size_t m = 0; m < members.size(); ++m)
            {
                if (members[m] != &client && notified.insert(members[m]->getFd()).second)
                    server.sendToClient(members[m]->getFd(), line);
            }
        }
    }

    client.setNickname(newNick);
    client.setNickOk(true);
    if (!wasRegistered && client.isRegistered())
        sendWelcome(client, server);
}
