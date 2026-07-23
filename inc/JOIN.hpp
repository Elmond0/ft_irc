#ifndef JOIN_HPP
#define JOIN_HPP

#include "ACommand.hpp"
#include <string>

class Channel;

class JOIN : public ACommand
{
	public:
		JOIN(Server &server);
		~JOIN(void);
		void execute(Client &client, const IrcMessage &msg);

	private:
		bool isValidChannelName(const std::string &name) const;
		void sendNames(Client &client, Channel &chan);
		void joinOne(Client &client, const std::string &name, const std::string &key);
		void partAll(Client &client);
};

#endif
