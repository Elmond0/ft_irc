#ifndef COMMANDS_HPP
# define COMMANDS_HPP

# include <string>
# include "IrcMessage.hpp"
# include "Client.hpp"
# include "Server.hpp"

# ifndef SERVER_NAME
#  define SERVER_NAME "ircserv"
# endif

/* comandi di registrazione */
void		handle_PASS(Client& client, const IrcMessage& msg, Server& server);
void		handle_NICK(Client& client, const IrcMessage& msg, Server& server);
void		handle_USER(Client& client, const IrcMessage& msg, Server& server);

/* helper condivisi (src/utils.cpp) */
std::string	nickOrStar(const Client& client);
void		sendWelcome(Client& client, Server& server);
bool		isValidNick(const std::string& nick);

#endif
