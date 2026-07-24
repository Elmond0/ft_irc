
#include "../inc/dispatch.hpp"
#include "../inc/CommandUtils.hpp"
#include "../inc/PASS.hpp"
#include "../inc/NICK.hpp"
#include "../inc/USER.hpp"
#include "../inc/JOIN.hpp"
#include "../inc/PRIVMSG.hpp"
#include "../inc/KICK.hpp"
#include "../inc/INVITE.hpp"
#include "../inc/TOPIC.hpp"
#include "../inc/MODE.hpp"
#include "../inc/QUIT.hpp"
#include "../inc/PING.hpp"
#include "../inc/PART.hpp"
#include <map>
#include <sstream>
#include <string>

Dispatcher::Dispatcher(Server& server) : _server(server)
{
    initCommands();
}

// Nota: non copiamo _commands (puntatori posseduti) per evitare una doppia
// delete. La copia riparte con la propria mappa di comandi.
Dispatcher::Dispatcher(const Dispatcher& other)
    : _server(other._server), _preReg(other._preReg)
{
    initCommands();
}

Dispatcher::~Dispatcher(void)
{
    for (std::map<std::string, ACommand*>::iterator it = _commands.begin();
         it != _commands.end(); ++it)
        delete it->second;
}

void Dispatcher::initCommands(void)
{
    _commands["PASS"]    = new PASS(_server);
    _commands["NICK"]    = new NICK(_server);
    _commands["USER"]    = new USER(_server);
    _commands["JOIN"]    = new JOIN(_server);
    _commands["PRIVMSG"] = new PRIVMSG(_server);
    _commands["KICK"]    = new KICK(_server);
    _commands["INVITE"]  = new INVITE(_server);
    _commands["TOPIC"]   = new TOPIC(_server);
    _commands["MODE"]    = new MODE(_server);
    _commands["QUIT"]    = new QUIT(_server);
    _commands["PING"]    = new PING(_server);
    _commands["PART"]    = new PART(_server);

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

    try
    {
        if (!client.isRegistered() && _preReg.find(msg.command) == _preReg.end())
            throw NotRegisteredException();

        std::map<std::string, ACommand*>::const_iterator it =
            _commands.find(msg.command);
        if (it == _commands.end())
            throw UnknownCommandException();

        it->second->execute(client, msg);
    }
    catch (const ACommand::NumericError& e)
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
