#ifndef KICK_HPP
#define KICK_HPP

#include "ACommand.hpp"

class KICK : public ACommand
{
	public:
		KICK(Server &server);
		~KICK(void);
		void execute(Client &client, const IrcMessage &msg);
};

#endif
