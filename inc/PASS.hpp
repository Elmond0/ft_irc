#ifndef PASS_HPP
#define PASS_HPP

#include "ACommand.hpp"

class PASS : public ACommand
{
	public:
		PASS(Server &server);
		~PASS(void);
		void execute(Client &client, const IrcMessage &msg);
};

#endif
