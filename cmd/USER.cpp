#include "../inc/Commands.hpp"

void USER(Client& client, const IrcMessage& msg, Server& server)
{
    if (client.isRegistered())
    {
        std::string reply = std::string(":") + SERVER_NAME + " 462 " +
            nickOrStar(client) + " :You may not reregister\r\n";
        server.sendToClient(client.getFd(), reply);
        return;
    }

    if (msg.params.empty() || msg.trailing.empty())
    {
        std::string reply = std::string(":") + SERVER_NAME + " 461 " +
            nickOrStar(client) + " USER :Not enough parameters\r\n";
        server.sendToClient(client.getFd(), reply);
        return;
    }

    client.setUser(msg.params[0]);
    client.setRealname(msg.trailing);
    client.setUserOk(true);

    if (client.isRegistered())
        sendWelcome(client, server);
}
