#ifndef PARSER_HPP
# define PARSER_HPP

# include <string>
# include "IrcMessage.hpp"

IrcMessage	parseMessage(const std::string& raw);

#endif
