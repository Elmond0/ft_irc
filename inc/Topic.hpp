#ifndef TOPIC_HPP
#define TOPIC_HPP

#include "ACommand.hpp"

class Topic : public ACommand {
public:
  Topic(Server &server);
  ~Topic(void);
  void execute(Client &client, const IrcMessage &msg);
};

#endif
