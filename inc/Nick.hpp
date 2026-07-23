#ifndef NICK_HPP
#define NICK_HPP

#include "ACommand.hpp"
#include <string>

class Nick : public ACommand {
public:
  Nick(Server &server);
  ~Nick(void);
  void execute(Client &client, const IrcMessage &msg);

private:
  bool isValidNick(const std::string &nick) const;
};

#endif
