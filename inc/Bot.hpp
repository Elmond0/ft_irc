#ifndef BOT_HPP
#define BOT_HPP

#include <string>

class Server;
class Client;
class Channel;

class Bot
{
	public:
		static bool	isBotNick(const std::string &nick);
		static void	onMessage(Server &server, Client &sender, Channel *chan, const std::string &text);

	private:
		Bot(void);
};

#endif
