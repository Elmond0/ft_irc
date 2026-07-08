#include "../inc/Commands.hpp"
#include <map>

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

void handle_NICK(Client& client, const IrcMessage& msg, Server& server)
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

    /* Il welcome va mandato UNA sola volta, quando la registrazione si
     * COMPLETA. Se il client era gia' registrato questo e' solo un cambio
     * nick a runtime: niente burst 001-004. */
    bool wasRegistered = client.isRegistered();
    client.setNickname(newNick);
    client.setNickOk(true);
    if (!wasRegistered && client.isRegistered())
        sendWelcome(client, server);
}
