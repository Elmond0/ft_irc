/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   dispatch.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: giomastr <giomastr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 15:37:29 by giomastr          #+#    #+#             */
/*   Updated: 2026/07/07 15:37:32 by giomastr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef DISPATCH_HPP
# define DISPATCH_HPP

# include <string>
# include <map>
# include <set>
# include <exception>
# include "IrcMessage.hpp"
# include "Client.hpp"
# include "Server.hpp"
# include "Commands.hpp"

class Dispatcher
{
	public:
		/* puntatore a un metodo-comando di Command */
		typedef void (Command::*CommandFn)(Client&, const IrcMessage&);

		Dispatcher(Server& server);
		Dispatcher(const Dispatcher& other);
		~Dispatcher(void);

		void	dispatch(Client& client, const IrcMessage& msg);

		class NotRegisteredException : public std::exception
		{
			public:
				const char*	what(void) const throw();
		};

		class UnknownCommandException : public std::exception
		{
			public:
				const char*	what(void) const throw();
		};

	private:
		Server&							_server;
		std::map<std::string, CommandFn>	_handlers;
		std::set<std::string>			_preReg; /* comandi leciti prima della registrazione */

		Dispatcher(void);

		void	initHandlers(void);
};

#endif
