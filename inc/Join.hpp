#ifndef JOIN_HPP
#define JOIN_HPP

#include "ACommand.hpp"
#include <string>

class Channel;

class Join : public ACommand {
public:
  Join(Server &server);
  ~Join(void);
  void execute(Client &client, const IrcMessage &msg);

private:
  bool isValidChannelName(const std::string &name) const;
  void sendNames(Client &client, Channel &chan);
  void joinOne(Client &client, const std::string &name, const std::string &key);
  void partAll(Client &client);
};

#endif
