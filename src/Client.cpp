#include "Client.hpp"

Client::Client( void ) : _passOk(false), _nickOk(false), _userOk(false) {}

Client::Client( int sock_fd, sockaddr_in clientAddress ) : _sock_fd(sock_fd), _addr(clientAddress), _isOp(false), _isChanOp(false), _passOk(false), _nickOk(false), _userOk(false) {}

Client::Client( Client const & other ) : _sock_fd(other._sock_fd), _isOp(other._isOp), _isChanOp(other._isChanOp), _nick(other._nick), _user(other._user), _realname(other._realname), _passOk(other._passOk), _nickOk(other._nickOk), _userOk(other._userOk) {}

Client& Client::operator=( Client const & other ) {
	if (this != &other) {
		_sock_fd = other._sock_fd;
		_nick = other._nick;
		_user = other._user;
		_realname = other._realname;
		_passOk = other._passOk;
		_nickOk = other._nickOk;
		_userOk = other._userOk;
	}
	return *this;
}

Client::~Client( void ) {}

int	Client::getSockFd( void ) const { return _sock_fd; }

sockaddr_in Client::getAddr( void ) const { return _addr; }

// interfaccia registrazione - @elia

int	Client::getFd( void ) const { return _sock_fd; }

const std::string&	Client::getNick( void ) const { return _nick; }

const std::string&	Client::getUser( void ) const { return _user; }

const std::string&	Client::getRealname( void ) const { return _realname; }

void	Client::setNick( const std::string& nick ) { _nick = nick; }

void	Client::setUser( const std::string& user ) { _user = user; }

void	Client::setRealname( const std::string& realname ) { _realname = realname; }

void	Client::setPassOk( bool ok ) { _passOk = ok; }

void	Client::setNickOk( bool ok ) { _nickOk = ok; }

void	Client::setUserOk( bool ok ) { _userOk = ok; }

bool	Client::isRegistered( void ) const { return _passOk && _nickOk && _userOk; }

std::ostream& operator<<( std::ostream& o, Client const & c ) {
	o << "CLIENT\nsocket fd: " << c.getSockFd() << "\naddress: " << inet_ntoa(c.getAddr().sin_addr) << "\n";
	return o;
}
