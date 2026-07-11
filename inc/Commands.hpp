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
# include <exception>
# include "IrcMessage.hpp"
# include "Client.hpp"
# include "Channel.hpp"
# include "Server.hpp"

# ifndef SERVER_NAME
#  define SERVER_NAME "ircserv"
# endif

class Command
{
	public:
		class NumericError : public std::exception
		{
			private:
				int			_code;
				std::string	_text;

			public:
				NumericError(int code, const std::string& text);
				virtual ~NumericError() throw();

				int					code() const;
				const std::string&	text() const;
				virtual const char*	what() const throw();
		};

		Command(Server& server);
		Command(const Command& other);
		~Command(void);

		void	PASS(Client& client, const IrcMessage& msg);
		void	NICK(Client& client, const IrcMessage& msg);
		void	USER(Client& client, const IrcMessage& msg);
	
		void	JOIN(Client& client, const IrcMessage& msg);
		void	PRIVMSG(Client& client, const IrcMessage& msg);
		void	KICK(Client& client, const IrcMessage& msg);
		void	INVITE(Client& client, const IrcMessage& msg);
		void	TOPIC(Client& client, const IrcMessage& msg);
		void	MODE(Client& client, const IrcMessage& msg);
		void	QUIT(Client& client, const IrcMessage& msg);
		void	PING(Client& client, const IrcMessage& msg);
		void	PART(Client& client, const IrcMessage& msg);

	private:
		Server&	_server;

		Command(void);

		void		numeric(Client& client, int code, const std::string& text);

		bool		isValidChannelName(const std::string& name) const;
		void		sendNames(Client& client, Channel& chan);
		void		joinOne(Client& client, const std::string& name,
						const std::string& key);
		void		partAll(Client& client);
		void		partOne(Client& client, const std::string& name,
						const std::string& reason);
		void		sendChannelModes(Client& client, Channel& chan);
		void		applyModes(Client& client, Channel& chan,
						const IrcMessage& msg);
		bool		applyOneMode(Client& client, Channel& chan, char c,
						bool adding, const IrcMessage& msg,
						std::size_t& argIdx, std::string& usedArg);
		static bool	nextArg(const IrcMessage& msg, std::size_t& idx,
						std::string& out);
};

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
