#ifndef QUIT_HPP
#define QUIT_HPP

#include "ACommand.hpp"

class QUIT : public ACommand
{
	public:
		QUIT(Server &server);
		~QUIT(void);
		void execute(Client &client, const IrcMessage &msg);
};

#endif
