#include "../inc/Commands.hpp"

std::string nickOrStar(const Client& client)
{
    if (client.getNickname().empty())
        return "*";
    return client.getNickname();
}

void sendWelcome(Client& client, Server& server)
{
    std::string nick = client.getNickname();

    server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME + " 001 " +
        nick + " :Welcome to the IRC Network " + nick + "!" + client.getUsername() +
        "@host\r\n");

    server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME + " 002 " +
        nick + " :Your host is " + SERVER_NAME + ", running version 1.0\r\n");

    server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME + " 003 " +
        nick + " :This server was created today\r\n");

    server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME + " 004 " +
        nick + " " + SERVER_NAME + " 1.0 o o\r\n");
}
