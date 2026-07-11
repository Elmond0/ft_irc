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

/*
** Command
** Tutti i comandi IRC raccolti in una classe: il dispatcher costruisce un
** Command con il riferimento al Server e invoca il metodo del comando
** ricevuto (tramite puntatore a metodo, vedi Dispatcher).
**
** Gli errori FATALI (quelli che troncano il comando) sono eccezioni
** NumericError catturate dal dispatcher, che spedisce il numeric al client
** (CPP05). Gli errori NON fatali (dentro i loop di JOIN/PART multipli o
** dei flag di MODE, dove si continua col prossimo elemento) usano invece
** il metodo numeric(), che spedisce subito senza interrompere.
*/
class Command
{
	public:
		/* errore fatale di un comando: codice numeric + resto della reply.
		** Il testo NON include nick e server: li aggiunge chi cattura. */
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

		/* fase 1 — registrazione */
		void	PASS(Client& client, const IrcMessage& msg);
		void	NICK(Client& client, const IrcMessage& msg);
		void	USER(Client& client, const IrcMessage& msg);

		/* fase 2 — chat base */
		void	JOIN(Client& client, const IrcMessage& msg);
		void	PRIVMSG(Client& client, const IrcMessage& msg);

		/* fase 3 — channel operator */
		void	KICK(Client& client, const IrcMessage& msg);
		void	INVITE(Client& client, const IrcMessage& msg);
		void	TOPIC(Client& client, const IrcMessage& msg);
		void	MODE(Client& client, const IrcMessage& msg);

		/* fase 4 — extra */
		void	QUIT(Client& client, const IrcMessage& msg);
		void	PING(Client& client, const IrcMessage& msg);
		void	PART(Client& client, const IrcMessage& msg);

	private:
		Server&	_server;

		Command(void);

		/* costruisce e spedisce ":SERVER code nick text\r\n" (CPP00:
		** ostringstream). Per errori non fatali e reply informative. */
		void		numeric(Client& client, int code, const std::string& text);

		/* helper interni */
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

/* helper condivisi (src/utils.cpp; isValidNick e' in src/cmd/NICK.cpp) */
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
