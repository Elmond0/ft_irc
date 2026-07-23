#ifndef INVITE_HPP
#define INVITE_HPP

#include "ACommand.hpp"

class INVITE : public ACommand 
{
	public:
		INVITE(Server &server);
		~INVITE(void);
		void execute(Client &client, const IrcMessage &msg);
};

#endif
