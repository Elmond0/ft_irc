#ifndef DISPATCH_HPP
# define DISPATCH_HPP

# include <string>
# include <exception>
# include "IrcMessage.hpp"
# include "Client.hpp"
# include "Server.hpp"

class Dispatcher
{
	private:
		Server&	_server;

		Dispatcher(void);

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
};

#endif
