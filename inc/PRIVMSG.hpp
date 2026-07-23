#ifndef PRIVMSG_HPP
#define PRIVMSG_HPP

#include "ACommand.hpp"

class PRIVMSG : public ACommand
{
	public:
		PRIVMSG(Server &server);
		~PRIVMSG(void);
		void execute(Client &client, const IrcMessage &msg);
};

#endif
