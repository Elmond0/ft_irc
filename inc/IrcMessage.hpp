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
