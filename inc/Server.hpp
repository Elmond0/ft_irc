#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <cstring>
# include <map>
# include <vector>
# include <sys/socket.h>
# include <netinet/in.h>
# include <poll.h>
# include <cerrno>

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

		void	addNewClient( std::vector<pollfd>& fds );
		std::string	readBuffer( pollfd pfd );

	public:
		Server( void );
		Server( int port, std::string );
		Server( Server const & other );
		Server& operator=( Server const & other );
		~Server( void );

		void	run( void );

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
};

#endif