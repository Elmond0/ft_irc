
#include "../inc/dispatch.hpp"
#include "../inc/Commands.hpp"
#include <sstream>



Dispatcher::Dispatcher(Server& server) : _server(server)
{
    initHandlers();
}

Dispatcher::Dispatcher(const Dispatcher& other)
    : _server(other._server), _handlers(other._handlers), _preReg(other._preReg) {}

Dispatcher::~Dispatcher(void) {}

void Dispatcher::initHandlers(void)
{
    _handlers["PASS"]    = &Command::PASS;
    _handlers["NICK"]    = &Command::NICK;
    _handlers["USER"]    = &Command::USER;
    _handlers["JOIN"]    = &Command::JOIN;
    _handlers["PRIVMSG"] = &Command::PRIVMSG;
    _handlers["KICK"]    = &Command::KICK;
    _handlers["INVITE"]  = &Command::INVITE;
    _handlers["TOPIC"]   = &Command::TOPIC;
    _handlers["MODE"]    = &Command::MODE;
    _handlers["QUIT"]    = &Command::QUIT;
    _handlers["PING"]    = &Command::PING;
    _handlers["PART"]    = &Command::PART;

    _preReg.insert("PASS");
    _preReg.insert("NICK");
    _preReg.insert("USER");
    _preReg.insert("PING");
    _preReg.insert("QUIT");
}

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

    Command cmd(_server);

    try
    {
        if (!client.isRegistered() && _preReg.find(msg.command) == _preReg.end())
            throw NotRegisteredException();

        std::map<std::string, CommandFn>::const_iterator it =
            _handlers.find(msg.command);
        if (it == _handlers.end())
            throw UnknownCommandException();

        (cmd.*(it->second))(client, msg);
    }
    catch (const Command::NumericError& e)
    {
        std::ostringstream oss;
        oss << ":" << SERVER_NAME << " " << e.code() << " "
            << nickOrStar(client) << " " << e.text() << "\r\n";
        _server.sendToClient(client.getFd(), oss.str());
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
