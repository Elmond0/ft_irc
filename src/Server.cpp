#include "Server.hpp"

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
	// bind(_listenSock_fd, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
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
				sockaddr_in	clientAddress;
				socklen_t len = sizeof(clientAddress);
				pollfd newClient;
				newClient.fd = accept(_listenSock_fd, (struct sockaddr*)&clientAddress, &len);
				newClient.events = POLLIN | POLLOUT;
				fds.push_back(newClient);
				_clients[newClient.fd] = Client(newClient.fd, clientAddress);

				std::cout << _clients[newClient.fd] << std::endl;
			}
			for (int i = 1; i < static_cast<int>(fds.size()); i++) {
				if (fds[i].revents & POLLIN) {
					// std::cout << "client " << i << " is POLLIN" << std::endl;
					char buffer[512];
					if (recv(fds[i].fd, &buffer, 512, 0) == -1)
				    	std::cerr << strerror(errno) << std::endl;
					std::cout << buffer << std::endl;
				}
			}
		}
	}
	
}

const char *Server::PortNotValid::what() const throw() { return "port not valid."; }

const char *Server::WrongPassword::what() const throw() { return "password incorrect. Try again."; }
