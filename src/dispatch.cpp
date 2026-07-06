
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
            else if (msg.command == "PING")
                handle_PING(client, msg, _server);
            else if (msg.command == "QUIT")
                handle_QUIT(client, msg, _server);
            else
                throw NotRegisteredException();
            return;
        }

        if (msg.command == "NICK")
            handle_NICK(client, msg, _server);
        else if (msg.command == "PASS")
            handle_PASS(client, msg, _server);
        else if (msg.command == "USER")
            handle_USER(client, msg, _server);
        else if (msg.command == "PING")
            handle_PING(client, msg, _server);
        else if (msg.command == "JOIN")
            handle_JOIN(client, msg, _server);
        else if (msg.command == "PRIVMSG")
            handle_PRIVMSG(client, msg, _server);
        else if (msg.command == "NOTICE")
            handle_NOTICE(client, msg, _server);
        else if (msg.command == "KICK")
            handle_KICK(client, msg, _server);
        else if (msg.command == "INVITE")
            handle_INVITE(client, msg, _server);
        else if (msg.command == "TOPIC")
            handle_TOPIC(client, msg, _server);
        else if (msg.command == "MODE")
            handle_MODE(client, msg, _server);
        else if (msg.command == "PART")
            handle_PART(client, msg, _server);
        else if (msg.command == "QUIT")
            handle_QUIT(client, msg, _server);
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
