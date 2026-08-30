#include "Server.hpp"
#include "Parser.hpp"
#include "dispatch.hpp"

extern volatile sig_atomic_t g_isRunning;

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
	std::map<int, Client>::iterator entry = _clients.find(fd);
	if (entry == _clients.end())
		throw ClientError();
	Client& c = entry->second;
	char	tmp[512];
	ssize_t		bytes = recv(fd, tmp, 512, 0);
	if (bytes < 0) {
		throw ClientError();
	}
	if (bytes == 0)
	{
		c.setQuitting();
		return (disconnectClient(c), c);
	}
	else if (bytes > 0) {
		std::string buffer = c.getRecvBuffer();
		buffer.append(tmp, bytes);
		size_t pos;
		while ((pos = buffer.find("\n")) != std::string::npos) {
			std::string line = buffer.substr(0, pos);
			buffer.erase(0 , pos + 1);
			IrcMessage msg = parseMessage(line);
			std::cout << msg << std::endl;
			Dispatcher dispatcher(*this);
				dispatcher.dispatch(c, msg);
		}
		if (c.getQuitting())
		{
			sendBuffer(fd);
			return (disconnectClient(c, 0);
		}
		c.setRecvBuffer(buffer);
	}
	return (bytes);
}

ssize_t	Server::sendBuffer( int fd ) {
	std::map<int, Client>::iterator entry = _clients.find(fd);
	if (entry == _clients.end())
		throw ClientError();
	Client& c = entry->second;
    size_t bytesSent = 0;
	std::string& buffer = c.getSendBuffer();
    size_t len = buffer.size();
    while (bytesSent < len) {
        ssize_t n = send(fd, buffer.c_str() + bytesSent, len - bytesSent, 0);
        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
				return 0;
			return n;
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
	if (fcntl(newClient.fd, F_SETFL, O_NONBLOCK) == -1)
		throw NetworkError();
	newClient.events = POLLIN | POLLOUT;
	_pfds.push_back(newClient);
	_clients[newClient.fd] = Client(newClient.fd, clientAddress);
	std::cout << _clients[newClient.fd] << std::endl;
}

void	Server::disconnectClient( Client c ) {
	for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
	{
		Channel chan = it->second;
		std::vector<Client *>::const_iterator itt = chan.getClients().find(c);
		if (itt != chan.getClients().end())
		{
			chan.removeClient(&c);
			if (chan.getClients().empty())
				_channels.erase(it);
		}
	}
	std::cout << "sockFd " << c.getFd() << " closed." << std::endl;
	if (close(c.getFd()) == -1)
		throw ClientError();
	for (std::list<pollfd>::iterator it = _pfds.begin(); it != _pfds.end(); ++it) {
		if ((*it).fd == c.getFd()) {
			_pfds.erase(it);
			return ;
		}
	}
	_clients.erase(c.getFd());
}

void	Server::disconnectAll( void ) {
	std::vector<int> fds;
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		fds.push_back(it->first);
	for (std::vector<int>::iterator it = fds.begin(); it != fds.end(); ++it) {
		std::map<int, Client>::iterator entry = _clients.find(it);
		if (entry == _clients.end())
			throw ClientError();
		Client& c = entry->second;
		disconnectClient(_clients.at(*it));
	}
}

void setQuitting( int sig ) { 
	(void)sig;
	g_isRunning = false;
}

void	Server::run( void ) {
	g_isRunning = true;
	struct sigaction sa;
    sa.sa_handler = setQuitting;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1)
		throw NetworkError();
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
		throw NetworkError();
	_listenSock_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listenSock_fd == -1)
		throw NetworkError();
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
	while (true) {
		std::vector<pollfd> vfds(_pfds.begin(), _pfds.end());
		int fdsNbr = poll(vfds.data(), vfds.size(), 1000);
		if (fdsNbr == -1)
			throw NetworkError();
		else {
			if (vfds[0].revents & POLLIN) {
				addNewClient();
			}
			for (std::vector<pollfd>::iterator it = vfds.begin() + 1; it != vfds.end(); ++it) {
				try
				{
					if (it->revents & POLLIN) {
						if (readBuffer(it->fd) < 0) {
							throw ClientError();
						}
					}
					if (!_clients[it->fd].getSendBuffer().empty()) {
						if (it->revents & POLLOUT) {
							ssize_t bytes = sendBuffer(it->fd);
							if (bytes < 0)
								throw ClientError();
							if (bytes == 0)
								continue ;
						}
					}
					if (it->revents & POLLERR) {
						throw ClientError();
					}
					if (it->revents & POLLHUP) {
						disconnectClient(_clients[it->fd]);
						continue ;
					}
				}
				catch(const std::exception& e)
				{
					disconnectClient(_clients[it->fd]);
					std::cerr << "ERROR: " << e.what() << '\n';
				}
			}
		}
		if (g_isRunning == false)
			break ;
	}
	shutdown();
}

void	Server::shutdown( void ) {
	disconnectAll();
}


// interfaccia per i comandi - @elia

const std::string&	Server::getPassword( void ) const { return _password; }

std::map<int, Client>&	Server::getClients( void ) { return _clients; }

std::map<std::string, Channel>&	Server::getChannels( void ) { return _channels; }

const char *Server::PortNotValid::what() const throw() { return "port not valid."; }

const char *Server::Timeout::what() const throw() { return "timed out."; }

const char *Server::NetworkError::what() const throw() { return strerror(errno); }

const char *Server::ClientError::what() const throw() { return strerror(errno); }
