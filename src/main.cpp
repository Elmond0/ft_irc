/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: miricci <miricci@student.42firenze.it>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/06/08 17:12:57 by miricci           #+#    #+#             */
/*   Updated: 2026/06/08 17:27:22 by miricci          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ircserv.h"

int	main( int ac, char **av ) {
	(void)av;
	if ( ac != 3 )
		throw std::runtime_error("Usage: ./ircserv <port> <password>");
	return 0;
}