#include "../../inc/User.hpp"
#include "../../inc/CommandUtils.hpp"

User::User(Server &server) : ACommand(server) {}

User::~User(void) {}

void User::execute(Client &client, const IrcMessage &msg)
{
  if (client.isRegistered())
    throw NumericError(462, ":You may not reregister");

  if (!client.isPassOk())
    throw NumericError(464, ":Password required");

  if (msg.params.empty() || msg.trailing.empty())
    throw NumericError(461, "USER :Not enough parameters");

  client.setUsername(msg.params[0]);
  client.setRealname(msg.trailing);
  client.setUserOk(true);

  finishRegistrationAttempt(client, _server);
}
