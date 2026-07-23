#ifndef TOPIC_HPP
#define TOPIC_HPP

#include "ACommand.hpp"

class TOPIC : public ACommand
{
	public:
		TOPIC(Server &server);
		~TOPIC(void);
		void execute(Client &client, const IrcMessage &msg);
};

#endif
