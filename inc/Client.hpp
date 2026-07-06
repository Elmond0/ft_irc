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

		bool _isOp;
		bool _isChanOp;

		std::string	_nick;
		std::string	_user;
		std::string	_realname;
		bool		_passOk;
		bool		_nickOk;
		bool		_userOk;

	public:
		Client( void );
		Client( int sock_fd, sockaddr_in clientAddress );
		Client( const Client& other );
		Client& operator=( const Client& other );
		~Client( void );

		int	getSockFd( void ) const;
		sockaddr_in getAddr( void ) const;

		int					getFd( void ) const;
		const std::string&	getNick( void ) const;
		const std::string&	getUser( void ) const;
		const std::string&	getRealname( void ) const;
		void				setNick( const std::string& nick );
		void				setUser( const std::string& user );
		void				setRealname( const std::string& realname );
		void				setPassOk( bool ok );
		void				setNickOk( bool ok );
		void				setUserOk( bool ok );
		bool				isRegistered( void ) const;
};

std::ostream& operator<<( std::ostream& o, Client const & c );

#endif