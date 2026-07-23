#ifndef MODE_HPP
#define MODE_HPP

#include "ACommand.hpp"
#include <cstddef>
#include <string>

class Channel;

class Mode : public ACommand {
public:
  Mode(Server &server);
  ~Mode(void);
  void execute(Client &client, const IrcMessage &msg);

private:
  void sendChannelModes(Client &client, Channel &chan);
  void applyModes(Client &client, Channel &chan, const IrcMessage &msg);
  bool applyOneMode(Client &client, Channel &chan, char c, bool adding, const IrcMessage &msg, std::size_t &argIdx,
                    std::string &usedArg);
  static bool nextArg(const IrcMessage &msg, std::size_t &idx, std::string &out);
};

#endif
