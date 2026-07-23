#include "Server.hpp"
#include "Parser.hpp"
#include "dispatch.hpp"

Server::Server( void ) {}

Server::Server( int port, std::string password ) : _port(port), _password(password) {}

Server::Server( Server const & other ) : _port(other._port), _password(other._password) {}

Server& Server::operator=( Server const & other ) {
	if (this != &other) {
		_port = other._port;
		_password = other._password;
		_listenSock_fd = other._listenSock_fd;
	}
	return *this;
}

Server::~Server( void ) {}

void	Server::readBuffer( int fd ) {
	int bytes;
	char	tmp[512];
	while (_clients[fd].getRecvBuffer().find("\r\n") != std::string::npos) {
		bytes = recv(fd, tmp, 512, 0);
		if (bytes < 0) {
			throw NetworkError();
		}
		else if (bytes == 0) {
			break ;
		}
		else {
			std::string buffer = _clients[fd].getRecvBuffer();
			buffer.append(tmp, bytes);
			// modifica buffer con setter

		}
	}
	std::cout << _clients[fd].getRecvBuffer() << std::endl;
}

void	Server::addNewClient( std::list<pollfd>& pfds ) {
	sockaddr_in	clientAddress;
	socklen_t len = sizeof(clientAddress);
	pollfd newClient;
	newClient.fd = accept(_listenSock_fd, (struct sockaddr*)&clientAddress, &len);
	newClient.events = POLLIN | POLLOUT;
	pfds.push_back(newClient);
	_clients[newClient.fd] = Client(newClient.fd, clientAddress);
	std::cout << _clients[newClient.fd] << std::endl;
}

void	Server::disconnectClient( std::list<pollfd>& pfds, pollfd cl) {
	std::cout << "sockFd " << cl.fd << " closed." << std::endl;
	if (close(cl.fd) == -1)
		throw NetworkError();
	_clients.erase(cl.fd);
	for (std::list<pollfd>::iterator it = pfds.begin(); it != pfds.end(); ++it) {
		if ((*it).fd == cl.fd) {
			pfds.erase(it);
			return ;
		}
	}
}
 
void	Server::run( void ) {
	if (_password != IRCPASS )
		throw WrongPassword();
	
	_listenSock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock_fd == -1)
		throw std::exception();
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(_port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	setsockopt(_listenSock_fd, SOL_SOCKET, SO_REUSEADDR, (void *)1, sizeof(SO_REUSEADDR));
	if (bind(_listenSock_fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		throw NetworkError();
	if (listen(_listenSock_fd, 5) == -1)
		throw NetworkError();
	std::list<pollfd> pfds;
	pollfd	serverPollfd;
	serverPollfd.fd = _listenSock_fd;
	serverPollfd.events = POLLIN;
	pfds.push_back(serverPollfd);
	while (true)
	{
		std::vector<pollfd> vfds(pfds.begin(), pfds.end());
		// for (std::list<pollfd>::iterator it = pfds.begin(); it != pfds.end(); ++it)
		// 	vfds.push_back(*it);
		int fdsNbr = poll(vfds.data(), vfds.size(), 1000);
		if (fdsNbr == -1)
			throw std::exception();
		else {
			if (vfds[0].revents == POLLIN) {
				addNewClient(pfds);
			}
			for (std::vector<pollfd>::iterator it = vfds.begin() + 1; it != vfds.end(); ++it) {
				if (it->revents & POLLIN) {
					readBuffer(it->fd);
					IrcMessage msg = parseMessage(_clients[it->fd].getRecvBuffer());
					Dispatcher dispatcher(*this);
					dispatcher.dispatch(_clients[it->fd], msg);
				}
				if (it->revents & POLLOUT) {
					//std::cout << fds[i].fd << ": POLLOUT" << std::cout;
				}
				if (it->revents & POLLERR) {
					disconnectClient(pfds, *it);
					std::cout << "POLLERR" << std::endl;
					throw NetworkError();
				}
				if (it->revents & POLLHUP) {
					std::cout << "POLLHUP" << std::endl;
					disconnectClient(pfds, *it);
				}
			}
		}
	}
}

// interfaccia per i comandi - @elia

const std::string&	Server::getPassword( void ) const { return _password; }

std::map<int, Client>&	Server::getClients( void ) { return _clients; }

void	Server::sendToClient( int fd, const std::string& msg ) {
	if (send(fd, msg.c_str(), msg.size(), 0) == -1)
		throw std::exception();
}

const char *Server::PortNotValid::what() const throw() { return "port not valid."; }

const char *Server::WrongPassword::what() const throw() { return "password incorrect. Try again."; }

const char *Server::Timeout::what() const throw() { return "timed out."; }

const char *Server::NetworkError::what() const throw() { return strerror(errno); }
