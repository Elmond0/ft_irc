#ifndef NICK_HPP
#define NICK_HPP

#include "ACommand.hpp"
#include <string>

class NICK : public ACommand
{
	public:
		NICK(Server &server);
		~NICK(void);
		void execute(Client &client, const IrcMessage &msg);

	private:
		bool isValidNick(const std::string &nick) const;
};

#endif
