#include "Server.hpp"

volatile sig_atomic_t g_isRunning = false;

void	parse_args(int argc, char **argv, int& port, std::string& password) {
	if (argc != 3)
		throw std::invalid_argument("Usage: ./ircserv <port> <password>");
	std::stringstream iss(argv[1]);
	char	leftover = 0;
	iss >> port;	iss >> leftover;
	if (leftover)
		throw std::invalid_argument("Usage: ./ircserv <port> <password>");
	password = argv[2];
}

int main(int argc, char **argv)
{
	int	port;
	std::string password;
	parse_args(argc, argv, port, password);
    Server server(port, password);
	try
	{
	    server.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << "ERROR: " << e.what() << '\n';
	}
    return 0;
}