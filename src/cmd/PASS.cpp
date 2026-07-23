#include "../../inc/PASS.hpp"
#include "../../inc/CommandUtils.hpp"

PASS::PASS(Server &server) : ACommand(server) {}

PASS::~PASS(void) {}

void PASS::execute(Client &client, const IrcMessage &msg)
{
  if (client.isRegistered())
    throw NumericError(462, ":You may not reregister");

  if (msg.params.empty())
    throw NumericError(461, "PASS :Not enough parameters");

  if (msg.params[0] != _server.getPassword())
    throw NumericError(464, ":Password incorrect");

  client.setPassOk(true);
  finishRegistrationAttempt(client, _server);
}
