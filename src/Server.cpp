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

std::string	Server::readBuffer( pollfd pfd ) {
	char buffer[512];
	if (recv(pfd.fd, &buffer, 512, 0) == -1)
		std::cerr << strerror(errno) << std::endl;
	std::cout << buffer << std::endl;
	std::string res(buffer);
	return (res);
}

void	Server::addNewClient( std::vector<pollfd>& fds ) {
	sockaddr_in	clientAddress;
	socklen_t len = sizeof(clientAddress);
	pollfd newClient;
	newClient.fd = accept(_listenSock_fd, (struct sockaddr*)&clientAddress, &len);
	newClient.events = POLLIN | POLLOUT;
	fds.push_back(newClient);
	_clients[newClient.fd] = Client(newClient.fd, clientAddress);
	std::cout << _clients[newClient.fd] << std::endl;
}
 
void	Server::run( void ) {
	if (_password != IRCPASS )
		throw WrongPassword();
	
	_listenSock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock_fd == -1)
		// errore
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(_port);
	std::cout << _port << std::endl;
	std::cout << serverAddress.sin_port << std::endl;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	setsockopt(_listenSock_fd, SOL_SOCKET, SO_REUSEADDR, (void *)1, sizeof(SO_REUSEADDR));
	if (bind(_listenSock_fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
    	std::cerr << strerror(errno) << std::endl;
	listen(_listenSock_fd, 5);
	std::vector<pollfd> fds;
	pollfd	serverPollfd;
	serverPollfd.fd = _listenSock_fd;
	serverPollfd.events = POLLIN;
	fds.push_back(serverPollfd);
	while (true)
	{
		if (poll(fds.data(), fds.size(), 1000)) {
			// std::cout << "polla" << std::endl;
			if (fds[0].revents == POLLIN) {
				addNewClient(fds);
			}
			for (int i = 1; i < static_cast<int>(fds.size()); i++) {
				if (fds[i].revents & POLLIN) {
					std::string raw = readBuffer(fds[i]);
					IrcMessage msg = parseMessage(raw);
					Dispatcher dispatcher(*this);
					dispatcher.dispatch(_clients[fds[i].fd], msg);
				}
				if (fds[i].revents & POLLOUT) {
					//std::cout << fds[i].fd << ": POLLOUT" << std::cout;
				}
				// if (fds[i].revents &
			}
		}
	}
}

// interfaccia per i comandi - @elia

const std::string&	Server::getPassword( void ) const { return _password; }

std::map<int, Client>&	Server::getClients( void ) { return _clients; }

std::map<std::string, Channel>&	Server::getChannels( void ) { return _channels; }

void	Server::sendToClient( int fd, const std::string& msg ) {
	if (send(fd, msg.c_str(), msg.size(), 0) == -1)
		std::cerr << strerror(errno) << std::endl;
}

const char *Server::PortNotValid::what() const throw() { return "port not valid."; }

const char *Server::WrongPassword::what() const throw() { return "password incorrect. Try again."; }
