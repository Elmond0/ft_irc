#ifndef USER_HPP
#define USER_HPP

#include "ACommand.hpp"

class User : public ACommand {
public:
  User(Server &server);
  ~User(void);
  void execute(Client &client, const IrcMessage &msg);
};

#endif
