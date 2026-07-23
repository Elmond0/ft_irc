/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: giomastr <giomastr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 15:37:17 by giomastr          #+#    #+#             */
/*   Updated: 2026/07/23 16:41:31 by giomastr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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

		// registrazione (PASS/NICK/USER) - @elia
		std::string	_nickname;
		std::string	_username;
		std::string _hostname;
		std::string	_realname;
		bool		_passOk;
		bool		_nickOk; //
		bool		_userOk; //
		bool		_registered;
		// buffer
		std::string _sendBuffer;
		std::string _recvBuffer;

	public:
		Client( void );
		Client( int sock_fd, sockaddr_in clientAddress );
		Client( const Client& other );
		Client& operator=( const Client& other );
		~Client( void );

		int	getSockFd( void ) const;
		sockaddr_in getAddr( void ) const;

		// interfaccia registrazione - @elia
		int					getFd( void ) const;

		const std::string&	getNickname( void ) const;
		const std::string&	getUsername( void ) const;
		const std::string&	getRealname( void ) const;
		const std::string&	getHostname( void ) const;

		void				setNickname( const std::string& nickname );
		void				setUsername( const std::string& username );
		void				setRealname( const std::string& realname );
		void				setHostname( const std::string& hostname );

		bool 				isPassOk() const;
		void				setPassOk( bool ok );
		void				setNickOk( bool ok );
		void				setUserOk( bool ok );
		bool				isRegistered( void ) const;

		// static message box
		std::string			getPrefix() const;

		void 				setSendBuffer(std::string sendBuffer);
		void 				setRecvBuffer(std::string recvBuffer);

		std::string			&getSendBuffer();
		std::string			&getRecvBuffer(); 		// Buffer di ricezione grezzo

		void				queueMessage(const std::string &message); 		// Accoda un messaggio gia' formattato (senza \r\n finale)
		void				clearSendBuffer();



};

std::ostream& operator<<( std::ostream& o, Client const & c );

#endif