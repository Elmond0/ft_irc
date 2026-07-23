#ifndef QUIT_HPP
#define QUIT_HPP

#include "ACommand.hpp"

class Quit : public ACommand {
public:
  Quit(Server &server);
  ~Quit(void);
  void execute(Client &client, const IrcMessage &msg);
};

#endif
