#ifndef IRCMESSAGE_HPP
# define IRCMESSAGE_HPP

# include <string>
# include <vector>

/* [:prefix] COMMAND [param1 param2 ...] [:trailing]\r\n */
struct IrcMessage
{
	std::string					prefix;
	std::string					command;
	std::vector<std::string>	params;
	std::string					trailing;
};

#endif
