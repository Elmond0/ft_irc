/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   IrcMessage.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: giomastr <giomastr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 15:37:06 by giomastr          #+#    #+#             */
/*   Updated: 2026/07/07 15:37:08 by giomastr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef IRCMESSAGE_HPP
# define IRCMESSAGE_HPP

# include <string>
# include <vector>
# include <ostream>

struct IrcMessage
{
	std::string					prefix;
	std::string					command;
	std::vector<std::string>	params;
	std::string					trailing;
};

std::ostream&	operator<<(std::ostream& o, const IrcMessage& msg);

#endif
