/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miricci <miricci@student.42firenze.it>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 15:37:53 by giomastr          #+#    #+#             */
/*   Updated: 2026/07/30 15:17:44 by miricci          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <cstring>
# include <sstream>
# include <map>
# include <list>
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <poll.h>
# include <cerrno>
# include <unistd.h>
# include <fcntl.h>
# include <signal.h>
# include <algorithm>

# include "Client.hpp"
# include "Channel.hpp"
# include "IrcMessage.hpp"

# define IRCPASS "dontpanic"

class Server
{
	private:
		int	_listenSock_fd;
		sockaddr_in serverAddress;
		int	_port;
		std::map<int, Client> 			_clients;
		std::list<pollfd>					_pfds;
		std::map<std::string, Channel> _channels;
		std::string _password;

		void		addNewClient( void );
		void		disconnectAll( void );
		ssize_t		readBuffer( int fd );
		ssize_t		sendBuffer( int fd );

	public:
		Server( void );
		Server( int port, std::string password );
		Server( Server const & other );
		Server& operator=( Server const & other );
		~Server( void );

		void	run( void );
		void	shutdown( void );
		void	disconnectClient( Client & client, const std::string & reason );

		// interfaccia per i comandi - @elia
		const std::string&				getPassword( void ) const;
		std::map<int, Client>&			getClients( void );
		std::map<std::string, Channel>&	getChannels( void );

		class PortNotValid : public std::exception
		{
			public:
				const char *what() const throw();
		};
		class Timeout : public std::exception
		{
			public:
				const char *what() const throw();
		};
		class NetworkError : public std::exception
		{
			public:
				const char *what() const throw();
		};
		class ClientError : public std::exception
		{
			public:
				const char *what() const throw();
		};
};

#endif