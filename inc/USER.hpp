#ifndef USER_HPP
#define USER_HPP

#include "ACommand.hpp"

class USER : public ACommand
{
	public:
		USER(Server &server);
		~USER(void);
		void execute(Client &client, const IrcMessage &msg);
};

#endif
