#ifndef INVITE_HPP
#define INVITE_HPP

#include "ACommand.hpp"

class Invite : public ACommand {
public:
  Invite(Server &server);
  ~Invite(void);
  void execute(Client &client, const IrcMessage &msg);
};

#endif
