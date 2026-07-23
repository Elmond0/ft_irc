#ifndef KICK_HPP
#define KICK_HPP

#include "ACommand.hpp"

class Kick : public ACommand {
public:
  Kick(Server &server);
  ~Kick(void);
  void execute(Client &client, const IrcMessage &msg);
};

#endif
