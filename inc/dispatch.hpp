#ifndef DISPATCH_HPP
# define DISPATCH_HPP

# include <string>
# include <map>
# include <set>
# include <exception>
# include "IrcMessage.hpp"
# include "Client.hpp"
# include "Server.hpp"
# include "ACommand.hpp"

class Dispatcher
{
	public:
		Dispatcher(Server& server);
		Dispatcher(const Dispatcher& other);
		~Dispatcher(void);

		void	dispatch(Client& client, const IrcMessage& msg);

		class NotRegisteredException : public std::exception
		{
			public:
				const char*	what(void) const throw();
		};

		class UnknownCommandException : public std::exception
		{
			public:
				const char*	what(void) const throw();
		};

	private:
		Server&							_server;
		std::map<std::string, ACommand*>	_commands;
		std::set<std::string>			_preReg;

		Dispatcher(void);

		void	initCommands(void);
};

#endif
