/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: giomastr <giomastr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/05 17:33:24 by giomastr          #+#    #+#             */
/*   Updated: 2026/07/23 16:41:22 by giomastr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client( void ) : _passOk(false), _nickOk(false), _userOk(false) {}

Client::Client( int sock_fd, sockaddr_in clientAddress ) : _sock_fd(sock_fd), _addr(clientAddress), _isOp(false), _isChanOp(false), _passOk(false), _nickOk(false), _userOk(false) {}

Client::Client( Client const & other ) : _sock_fd(other._sock_fd), _isOp(other._isOp), _isChanOp(other._isChanOp), _nickname(other._nickname), _username(other._username), _realname(other._realname), _passOk(other._passOk), _nickOk(other._nickOk), _userOk(other._userOk) {}

Client& Client::operator=( Client const & other ) {
	if (this != &other) {
		_sock_fd = other._sock_fd;
		_nickname = other._nickname;
		_username = other._username;
		_realname = other._realname;
		_hostname = other._hostname;
		_passOk = other._passOk;
		_nickOk = other._nickOk;
		_userOk = other._userOk;
		_registered = other._registered;
		_sendBuffer = other._sendBuffer;
		_recvBuffer = other._recvBuffer;
	}
	return *this;
}

Client::~Client( void ) {}

int	Client::getSockFd( void ) const { return _sock_fd; }

sockaddr_in Client::getAddr( void ) const { return _addr; }

// interfaccia registrazione - @elia

int	Client::getFd( void ) const { return _sock_fd; }

const std::string&	Client::getNickname( void ) const { return _nickname; }

const std::string&	Client::getUsername( void ) const { return _username; }

const std::string&	Client::getRealname( void ) const { return _realname; }

const std::string&	Client::getHostname( void ) const { return _hostname; }


void	Client::setNickname( const std::string& nickname ) { _nickname = nickname; }

void	Client::setUsername( const std::string& username ) { _username = username; }

void	Client::setRealname( const std::string& realname ) { _realname = realname; }

void	Client::setHostname( const std::string& hostname ) { _hostname = hostname; }

bool	Client::isPassOk( void ) const { return _passOk; } // getter necessario

void	Client::setPassOk( bool ok ) { _passOk = ok; }

void	Client::setNickOk( bool ok ) { _nickOk = ok; }

void	Client::setUserOk( bool ok ) { _userOk = ok; }

bool	Client::isRegistered( void ) const { return _passOk && _nickOk && _userOk; }

// message handling
std::string Client::getPrefix() const {
	std::string host;

	if(_hostname.empty())
		host = "localhost";
	else
		host = _hostname;
	return (_nickname + "!" + _username + "@" + host);
}

//buffer

void Client::queueMessage(const std::string &message)
{
	_sendBuffer += message;
	_sendBuffer += "\r\n";
}

void Client::clearSendBuffer()
{
	_sendBuffer.clear();
}

void	Client::setSendBuffer( std::string sendBuffer ) { _sendBuffer = sendBuffer; }

void	Client::setRecvBuffer( std::string recvBuffer ) { _recvBuffer = recvBuffer; }

std::string &Client::getSendBuffer()
{
	return (_sendBuffer);
}

std::string &Client::getRecvBuffer()
{
	return (_recvBuffer);
}



std::ostream& operator<<( std::ostream& o, Client const & c ) {
	o << "CLIENT\nsocket fd: " << c.getSockFd() << "\naddress: " << inet_ntoa(c.getAddr().sin_addr) << "\n";
	return o;
}
