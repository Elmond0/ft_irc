#ifndef COMMANDUTILS_HPP
#define COMMANDUTILS_HPP

#include "Channel.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include <string>
#include <vector>

#ifndef SERVER_NAME
#define SERVER_NAME "ircserv"
#endif


std::string					nickOrStar(const Client &client);
void						sendWelcome(Client &client, Server &server);
void						rejectRegistration(Client &client, Server &server, int code, const std::string &text);
void						finishRegistrationAttempt(Client &client, Server &server);
bool						sameNick(const std::string &a, const std::string &b);
std::string					userPrefix(const Client &client);
Client						*findClientByNick(Server &server, const std::string &nick);
Channel						*findChannel(Server &server, const std::string &name);
void						broadcastToChannel(Server &server, Channel &chan, const std::string &line, int exceptFd);
std::vector<std::string>	splitComma(const std::string &s);

#endif
