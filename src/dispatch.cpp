
#include "../inc/dispatch.hpp"
#include "../inc/Commands.hpp"



Dispatcher::Dispatcher(Server& server) : _server(server) {}

Dispatcher::Dispatcher(const Dispatcher& other) : _server(other._server) {}

Dispatcher::~Dispatcher(void) {}


const char* Dispatcher::NotRegisteredException::what(void) const throw()
{
    return ("You have not registered");
}

const char* Dispatcher::UnknownCommandException::what(void) const throw()
{
    return ("Unknown command");
}

void Dispatcher::dispatch(Client& client, const IrcMessage& msg)
{
    if (msg.command.empty())
        return;

    try
    {
        if (!client.isRegistered())
        {
            if (msg.command == "PASS")
                handle_PASS(client, msg, _server);
            else if (msg.command == "NICK")
                handle_NICK(client, msg, _server);
            else if (msg.command == "USER")
                handle_USER(client, msg, _server);
            else if (msg.command == "CAP")
                return;
            else
                throw NotRegisteredException();
            return;
        }

        if (msg.command == "NICK")
            handle_NICK(client, msg, _server);
        else if (msg.command == "PASS")
            handle_PASS(client, msg, _server); /* -> 462 gia' registrato */
        else if (msg.command == "USER")
            handle_USER(client, msg, _server); /* -> 462 gia' registrato */
        else if (msg.command == "PING")
            (void)0; /* handle_PING -- da aggiungere */
        else if (msg.command == "JOIN")
            (void)0; /* handle_JOIN -- da aggiungere */
        else if (msg.command == "PRIVMSG")
            (void)0; /* handle_PRIVMSG -- da aggiungere */
        else if (msg.command == "KICK")
            (void)0; /* handle_KICK -- da aggiungere */
        else if (msg.command == "INVITE")
            (void)0; /* handle_INVITE -- da aggiungere */
        else if (msg.command == "TOPIC")
            (void)0; /* handle_TOPIC -- da aggiungere */
        else if (msg.command == "MODE")
          (void)0; /* handle_MODE -- da aggiungere */
        else if (msg.command == "QUIT")
            (void)0; /* handle_QUIT -- da aggiungere */
        else
            throw UnknownCommandException();
    }
    catch (const NotRegisteredException& e)
    {
        std::string reply = std::string(":") + SERVER_NAME + " 451 " +
            nickOrStar(client) + " :" + e.what() + "\r\n";
        _server.sendToClient(client.getFd(), reply);
    }
    catch (const UnknownCommandException& e)
    {
        std::string reply = std::string(":") + SERVER_NAME + " 421 " +
            nickOrStar(client) + " " + msg.command +
            " :" + e.what() + "\r\n";
        _server.sendToClient(client.getFd(), reply);
    }
}
