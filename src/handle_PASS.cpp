#include "../inc/Commands.hpp"

void handle_PASS(Client& client, const IrcMessage& msg, Server& server)
{
    if (client.isRegistered())
    {
        std::string reply = std::string(":") + SERVER_NAME + " 462 " +
            nickOrStar(client) + " :You may not reregister\r\n";
        server.sendToClient(client.getFd(), reply);
        return;
    }

    if (msg.params.empty())
    {
        std::string reply = std::string(":") + SERVER_NAME + " 461 " +
            nickOrStar(client) + " PASS :Not enough parameters\r\n";
        server.sendToClient(client.getFd(), reply);
        return;
    }

    if (msg.params[0] != server.getPassword())
    {
        std::string reply = std::string(":") + SERVER_NAME + " 464 " +
            nickOrStar(client) + " :Password incorrect\r\n";
        server.sendToClient(client.getFd(), reply);
        return;
    }

    client.setPassOk(true);
}
