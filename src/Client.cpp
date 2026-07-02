#include "Client.hpp"

Client::Client( void ) {}

Client::Client( int sock_fd, sockaddr_in clientAddress ) : _sock_fd(sock_fd), _addr(clientAddress), _isOp(false), _isChanOp(false) {}

Client::Client( Client const & other ) : _sock_fd(other._sock_fd), _isOp(other._isOp), _isChanOp(other._isChanOp) {}

Client& Client::operator=( Client const & other ) {
	if (this != &other) {
		_sock_fd = other._sock_fd;
	}
	return *this;
}

Client::~Client( void ) {}

int	Client::getSockFd( void ) const { return _sock_fd; }

sockaddr_in Client::getAddr( void ) const { return _addr; }

std::ostream& operator<<( std::ostream& o, Client const & c ) {
	o << "CLIENT\nsocket fd: " << c.getSockFd() << "\naddress: " << inet_ntoa(c.getAddr().sin_addr) << "\n";
	return o;
}
