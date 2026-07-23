#ifndef PING_HPP
#define PING_HPP

#include "ACommand.hpp"

class Ping : public ACommand {
public:
  Ping(Server &server);
  ~Ping(void);
  void execute(Client &client, const IrcMessage &msg);
};

#endif
