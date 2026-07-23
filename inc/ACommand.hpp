#ifndef ACOMMAND_HPP
#define ACOMMAND_HPP

#include "Client.hpp"
#include "IrcMessage.hpp"
#include "Server.hpp"
#include <exception>
#include <string>

#ifndef SERVER_NAME
#define SERVER_NAME "ircserv"
#endif

class ACommand {
public:

  class NumericError : public std::exception {
  private:
    int _code;
    std::string _text;

  public:
    NumericError(int code, const std::string &text);
    virtual ~NumericError() throw();

    int code() const;
    const std::string &text() const;
    virtual const char *what() const throw();
  };

  ACommand(Server &server);
  virtual ~ACommand(void);

  virtual void execute(Client &client, const IrcMessage &msg) = 0;

protected:
  Server &_server;

  void numeric(Client &client, int code, const std::string &text);

private:
  ACommand(void);
};

#endif
