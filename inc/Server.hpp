/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: giomastr <giomastr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 15:37:53 by giomastr          #+#    #+#             */
/*   Updated: 2026/07/07 15:37:57 by giomastr         ###   ########.fr       */
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

# include "Client.hpp"

# define IRCPASS "dontpanic"

class Server
{
	private:
		int	_listenSock_fd;
		sockaddr_in serverAddress;
		int	_port;
		std::map<int, Client> _clients;
		std::string _password;

		void	addNewClient( std::list<pollfd>& fds );
		void	disconnectClient( std::list<pollfd>& fds, pollfd cl );
		void	readBuffer( int fd );

	public:
		Server( void );
		Server( int port, std::string );
		Server( Server const & other );
		Server& operator=( Server const & other );
		~Server( void );

		void	run( void );

		// interfaccia per i comandi - @elia
		const std::string&		getPassword( void ) const;
		std::map<int, Client>&	getClients( void );
		void					sendToClient( int fd, const std::string& msg );

		class WrongPassword : public std::exception
		{
			public:
				const char *what() const throw();
		};

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
};

#endif