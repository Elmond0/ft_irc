#include "../../inc/PING.hpp"

PING::PING(Server &server) : ACommand(server) {}

PING::~PING(void) {}

void PING::execute(Client &client, const IrcMessage &msg)
{
    std::string token = msg.trailing;
    if (token.empty() && !msg.params.empty())
        token = msg.params[0];

    if (token.empty())
        throw NumericError(409, ":No origin specified");

    _server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME +
        " PONG " + SERVER_NAME + " :" + token + "\r\n");
}
