#ifndef PART_HPP
#define PART_HPP

#include "ACommand.hpp"
#include <string>

class Part : public ACommand {
public:
  Part(Server &server);
  ~Part(void);
  void execute(Client &client, const IrcMessage &msg);

private:
  void partOne(Client &client, const std::string &name, const std::string &reason);
};

#endif
