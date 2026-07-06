#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include <string>
# include <vector>
# include "IrcMessage.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "Server.hpp"

# ifndef SERVER_NAME
#  define SERVER_NAME "ircserv"
# endif

/* fase 1 — registrazione */
void		handle_PASS(Client& client, const IrcMessage& msg, Server& server);
void		handle_NICK(Client& client, const IrcMessage& msg, Server& server);
void		handle_USER(Client& client, const IrcMessage& msg, Server& server);

/* fase 2 — chat base */
void		handle_JOIN(Client& client, const IrcMessage& msg, Server& server);
void		handle_PRIVMSG(Client& client, const IrcMessage& msg, Server& server);

/* fase 3 — channel operator */
void		handle_KICK(Client& client, const IrcMessage& msg, Server& server);
void		handle_INVITE(Client& client, const IrcMessage& msg, Server& server);
void		handle_TOPIC(Client& client, const IrcMessage& msg, Server& server);
void		handle_MODE(Client& client, const IrcMessage& msg, Server& server);

/* fase 4 — extra */
void		handle_QUIT(Client& client, const IrcMessage& msg, Server& server);
void		handle_PING(Client& client, const IrcMessage& msg, Server& server);
void		handle_PART(Client& client, const IrcMessage& msg, Server& server);
void		handle_NOTICE(Client& client, const IrcMessage& msg, Server& server);

/* helper condivisi (src/utils.cpp) */
std::string	nickOrStar(const Client& client);
void		sendWelcome(Client& client, Server& server);
bool		isValidNick(const std::string& nick);
std::string	userPrefix(const Client& client);
Client*		findClientByNick(Server& server, const std::string& nick);
Channel*	findChannel(Server& server, const std::string& name);
void		broadcastToChannel(Server& server, Channel& chan,
				const std::string& line, int exceptFd);
std::vector<std::string>	splitComma(const std::string& s);

#endif
