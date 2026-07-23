#ifndef PING_HPP
#define PING_HPP

#include "ACommand.hpp"

class PING : public ACommand
{
	public:
		PING(Server &server);
		~PING(void);
		void execute(Client &client, const IrcMessage &msg);
};

#endif
