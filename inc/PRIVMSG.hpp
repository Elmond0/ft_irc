#ifndef PRIVMSG_HPP
#define PRIVMSG_HPP

#include "ACommand.hpp"
#include <string>

class PRIVMSG : public ACommand
{
	public:
		PRIVMSG(Server &server);
		~PRIVMSG(void);
		void execute(Client &client, const IrcMessage &msg);

	private:
		void sendOne(Client &client, const std::string &target, const std::string &text);
};

#endif
