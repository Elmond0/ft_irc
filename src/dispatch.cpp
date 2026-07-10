
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
                PASS(client, msg, _server);
            else if (msg.command == "NICK")
                NICK(client, msg, _server);
            else if (msg.command == "USER")
                USER(client, msg, _server);
            else if (msg.command == "PING")
                PING(client, msg, _server);
            else if (msg.command == "QUIT")
                QUIT(client, msg, _server);
            else
                throw NotRegisteredException();
            return;
        }

        if (msg.command == "NICK")
            NICK(client, msg, _server);
        else if (msg.command == "PASS")
            PASS(client, msg, _server);
        else if (msg.command == "USER")
            USER(client, msg, _server);
        else if (msg.command == "PING")
            PING(client, msg, _server);
        else if (msg.command == "JOIN")
            JOIN(client, msg, _server);
        else if (msg.command == "PRIVMSG")
            PRIVMSG(client, msg, _server);
        else if (msg.command == "NOTICE")
            ; /* non implementato; ignorato in silenzio: la RFC vieta
                 di rispondere a un NOTICE, quindi niente 421 */
        else if (msg.command == "KICK")
            KICK(client, msg, _server);
        else if (msg.command == "INVITE")
            INVITE(client, msg, _server);
        else if (msg.command == "TOPIC")
            TOPIC(client, msg, _server);
        else if (msg.command == "MODE")
            MODE(client, msg, _server);
        else if (msg.command == "PART")
            PART(client, msg, _server);
        else if (msg.command == "QUIT")
            QUIT(client, msg, _server);
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
