/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: giomastr <giomastr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/30 17:36:24 by giomastr          #+#    #+#             */
/*   Updated: 2026/07/30 17:37:38 by giomastr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

void	parse_args(int argc, char **argv, int& port, std::string& password) {
	if (argc != 3)
		throw std::invalid_argument("Usage: ./ircserv <port> <password>");
	std::stringstream iss(argv[1]);
	char	leftover = 0; // da inizializzare
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