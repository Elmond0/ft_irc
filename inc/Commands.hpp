#ifndef COMMANDS_HPP
#define COMMANDS_HPP

#include "ACommand.hpp"
#include <cstddef>
#include <string>

class Channel;

class Pass : public ACommand {
public:
  Pass(Server &server);
  ~Pass(void);
  void execute(Client &client, const IrcMessage &msg);
};

class Nick : public ACommand {
public:
  Nick(Server &server);
  ~Nick(void);
  void execute(Client &client, const IrcMessage &msg);

private:
  bool isValidNick(const std::string &nick) const;
};

class User : public ACommand {
public:
  User(Server &server);
  ~User(void);
  void execute(Client &client, const IrcMessage &msg);
};

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

class Privmsg : public ACommand {
public:
  Privmsg(Server &server);
  ~Privmsg(void);
  void execute(Client &client, const IrcMessage &msg);
};

class Kick : public ACommand {
public:
  Kick(Server &server);
  ~Kick(void);
  void execute(Client &client, const IrcMessage &msg);
};

class Invite : public ACommand {
public:
  Invite(Server &server);
  ~Invite(void);
  void execute(Client &client, const IrcMessage &msg);
};

class Topic : public ACommand {
public:
  Topic(Server &server);
  ~Topic(void);
  void execute(Client &client, const IrcMessage &msg);
};

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

class Quit : public ACommand {
public:
  Quit(Server &server);
  ~Quit(void);
  void execute(Client &client, const IrcMessage &msg);
};

class Ping : public ACommand {
public:
  Ping(Server &server);
  ~Ping(void);
  void execute(Client &client, const IrcMessage &msg);
};

class Part : public ACommand {
public:
  Part(Server &server);
  ~Part(void);
  void execute(Client &client, const IrcMessage &msg);

private:
  void partOne(Client &client, const std::string &name, const std::string &reason);
};

#endif
