#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include "IrcMessage.hpp"

/* riceve UNA riga completa (senza accumulo pacchetti: quello lo fa la rete)
 * e la spezza in prefix / command (MAIUSCOLO) / params / trailing */
IrcMessage	parseMessage(const std::string& raw);

#endif
