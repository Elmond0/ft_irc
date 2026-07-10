#include "../inc/Commands.hpp"

void PING(Client& client, const IrcMessage& msg, Server& server)
{
    std::string token = msg.trailing;
    if (token.empty() && !msg.params.empty())
        token = msg.params[0];

    if (token.empty())
    {
        server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
            " 409 " + nickOrStar(client) + " :No origin specified\r\n");
        return;
    }

    server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
        " PONG " + SERVER_NAME + " :" + token + "\r\n");
}
