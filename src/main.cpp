#include "Server.hpp"

void	parse_args(int argc, char **argv, int& port, std::string& password) {
	if (argc != 3)
		throw std::invalid_argument("Usage: ./ircserv <port> <password>");
	std::stringstream iss(argv[1]);
	char	leftover;
	iss >> port;	iss >> leftover;
	if (leftover)
		throw std::invalid_argument("Usage: ./ircserv <port> <password>");
	password = argv[2];	
}

int main(int argc, char **argv)
{
	int	port;
	std::string password;
	try
	{
		parse_args(argc, argv, port, password);
    	Server server(port, password);
	    server.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << "ERROR: " << e.what() << '\n';
	}
    return 0;
}