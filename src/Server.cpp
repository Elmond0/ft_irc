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

ssize_t	Server::readBuffer( int fd ) {
	char	tmp[512];
	ssize_t		bytes = recv(fd, tmp, 512, 0);
	if (bytes < 0) {
		throw NetworkError();
	}
	else if (bytes > 0) {
		std::string buffer = _clients[fd].getRecvBuffer();
		buffer.append(tmp, bytes);
		size_t pos;
		while ((pos = buffer.find("\n")) != std::string::npos) {
			std::string line = buffer.substr(0, pos);
			buffer.erase(0 , pos + 1);
			if (!buffer.empty() && buffer[buffer.size() - 1] == '\r')
    			line.erase(line.size() - 1);
				IrcMessage msg = parseMessage(line);
				std::cout << msg << std::endl;
				Dispatcher dispatcher(*this);
				dispatcher.dispatch(_clients[fd], msg);
		}
		if (_clients[fd].getQuitting())
			return (disconnectClient(fd), 0);
		_clients[fd].setRecvBuffer(buffer);
	}
	return (bytes);
}

ssize_t	Server::sendBuffer( int fd ) {
    size_t bytesSent = 0;
	std::string& buffer = _clients[fd].getSendBuffer();
    size_t len = buffer.size();
    while (bytesSent < len) {
        ssize_t n = send(fd, buffer.c_str() + bytesSent, len - bytesSent, 0);
        if (n < 0) {
            return (n);
        } else if (n == 0)
			break ;
        bytesSent += n;
    }
	if (bytesSent > 0)
		buffer.erase(0, bytesSent);
	return (bytesSent);
}

void	Server::addNewClient() {
	sockaddr_in	clientAddress;
	socklen_t len = sizeof(clientAddress);
	pollfd newClient;
	newClient.fd = accept(_listenSock_fd, (struct sockaddr*)&clientAddress, &len);
	if (newClient.fd == -1)
		throw NetworkError();
	if (fcntl(_listenSock_fd, F_SETFL, O_NONBLOCK) == -1)
		throw NetworkError();
	newClient.events = POLLIN | POLLOUT;
	_pfds.push_back(newClient);
	_clients[newClient.fd] = Client(newClient.fd, clientAddress);
	std::cout << _clients[newClient.fd] << std::endl;
}

void	Server::disconnectClient( int fd ) {
	std::cout << "sockFd " << fd << " closed." << std::endl;
	if (close(fd) == -1)
		throw NetworkError();
	if (_clients.erase(fd) == 0)
		std::cout << "Client non trovato" << std::endl;
	for (std::list<pollfd>::iterator it = _pfds.begin(); it != _pfds.end(); ++it) {
		if ((*it).fd == fd) {
			_pfds.erase(it);
			return ;
		}
	}
}

void	Server::run( void ) {
	_listenSock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock_fd == -1)
		throw std::exception();
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(_port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	int opt = 1;
	setsockopt(_listenSock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(SO_REUSEADDR));
	if (bind(_listenSock_fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
		throw NetworkError();
	if (fcntl(_listenSock_fd, F_SETFL, O_NONBLOCK) == -1)
		throw NetworkError();
	if (listen(_listenSock_fd, 5) == -1)
		throw NetworkError();
	pollfd	serverPollfd;
	serverPollfd.fd = _listenSock_fd;
	serverPollfd.events = POLLIN;
	_pfds.push_back(serverPollfd);
	while (true)
	{
		int i = 0;
		std::vector<pollfd> vfds(_pfds.begin(), _pfds.end());
		int fdsNbr = poll(vfds.data(), vfds.size(), 1000);
		if (fdsNbr == -1)
			throw std::exception();
		else {
			if (vfds[0].revents == POLLIN) {
				addNewClient();
			}
			for (std::vector<pollfd>::iterator it = vfds.begin() + 1; it != vfds.end(); ++it) {
				if (it->revents & POLLIN)
					if (readBuffer(it->fd) < 0) {
						disconnectClient(it->fd);
						throw NetworkError();
					}
				if (it->revents & POLLOUT) {
					if (sendBuffer(it->fd) < 0) {
						disconnectClient(it->fd);
						throw NetworkError();
					}
				}
				if (it->revents & POLLERR) {
					disconnectClient(it->fd);
					std::cout << "POLLERR" << std::endl;
					throw NetworkError();
				}
				if (it->revents & POLLHUP) {
					std::cout << "POLLHUP" << std::endl;
					disconnectClient(it->fd);
					break ;
				}
			}
			i++;
		}
	}
}

// interfaccia per i comandi - @elia

const std::string&	Server::getPassword( void ) const { return _password; }

std::map<int, Client>&	Server::getClients( void ) { return _clients; }

std::map<std::string, Channel>&	Server::getChannels( void ) { return _channels; }

const char *Server::PortNotValid::what() const throw() { return "port not valid."; }

const char *Server::WrongPassword::what() const throw() { return "password incorrect. Try again."; }

const char *Server::Timeout::what() const throw() { return "timed out."; }

const char *Server::NetworkError::what() const throw() { return strerror(errno); }
