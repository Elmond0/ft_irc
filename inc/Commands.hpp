/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Commands.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: giomastr <giomastr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 15:37:25 by giomastr          #+#    #+#             */
/*   Updated: 2026/07/07 15:37:26 by giomastr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


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
void		PASS(Client& client, const IrcMessage& msg, Server& server);
void		NICK(Client& client, const IrcMessage& msg, Server& server);
void		USER(Client& client, const IrcMessage& msg, Server& server);

/* fase 2 — chat base */
void		JOIN(Client& client, const IrcMessage& msg, Server& server);
void		PRIVMSG(Client& client, const IrcMessage& msg, Server& server);

/* fase 3 — channel operator */
void		KICK(Client& client, const IrcMessage& msg, Server& server);
void		INVITE(Client& client, const IrcMessage& msg, Server& server);
void		TOPIC(Client& client, const IrcMessage& msg, Server& server);
void		MODE(Client& client, const IrcMessage& msg, Server& server);

/* fase 4 — extra */
void		QUIT(Client& client, const IrcMessage& msg, Server& server);
void		PING(Client& client, const IrcMessage& msg, Server& server);
void		PART(Client& client, const IrcMessage& msg, Server& server);

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
