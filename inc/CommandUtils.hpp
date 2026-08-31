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

#ifndef NETWORK_NAME
#define NETWORK_NAME "ft_irc"
#endif

#ifndef SERVER_VERSION
#define SERVER_VERSION "1.0"
#endif

#define USER_MODES "-"
#define CHANNEL_MODES "itkol"


std::string					nickOrStar(const Client &client);
void						sendWelcome(Client &client);
void						finishRegistrationAttempt(Client &client);
bool						sameNick(const std::string &a, const std::string &b);
std::string					userPrefix(const Client &client);
Client						*findClientByNick(Server &server, const std::string &nick);
Channel						*findChannel(Server &server, const std::string &name);
std::vector<std::string>	splitComma(const std::string &s);

#endif
