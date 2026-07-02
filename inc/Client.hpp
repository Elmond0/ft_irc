#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <iostream>
# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>

class Client
{
	private:
		int	_sock_fd;
		sockaddr_in	_addr;

		// permessi
		bool _isOp;
		bool _isChanOp;

	public:
		Client( void );
		Client( int sock_fd, sockaddr_in clientAddress );
		Client( const Client& other );
		Client& operator=( const Client& other );
		~Client( void );

		int	getSockFd( void ) const;
		sockaddr_in getAddr( void ) const;
};

std::ostream& operator<<( std::ostream& o, Client const & c );

#endif