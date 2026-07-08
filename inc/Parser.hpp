/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: giomastr <giomastr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 15:37:46 by giomastr          #+#    #+#             */
/*   Updated: 2026/07/07 15:37:47 by giomastr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include "IrcMessage.hpp"

/* riceve UNA riga completa (senza accumulo pacchetti: quello lo fa la rete)
 * e la spezza in prefix / command (MAIUSCOLO) / params / trailing */
IrcMessage	parseMessage(const std::string& raw);

#endif
