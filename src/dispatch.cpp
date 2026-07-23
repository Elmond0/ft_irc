
#include "../inc/dispatch.hpp"
#include "../inc/CommandUtils.hpp"
#include "../inc/Pass.hpp"
#include "../inc/Nick.hpp"
#include "../inc/User.hpp"
#include "../inc/Join.hpp"
#include "../inc/Privmsg.hpp"
#include "../inc/Kick.hpp"
#include "../inc/Invite.hpp"
#include "../inc/Topic.hpp"
#include "../inc/Mode.hpp"
#include "../inc/Quit.hpp"
#include "../inc/Ping.hpp"
#include "../inc/Part.hpp"
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

// Ogni comando e' una classe concreta derivata da ACommand (come i Form
// dell'ex02). Qui vengono istanziati una volta e associati al loro nome.
void Dispatcher::initCommands(void)
{
    _commands["PASS"]    = new Pass(_server);
    _commands["NICK"]    = new Nick(_server);
    _commands["USER"]    = new User(_server);
    _commands["JOIN"]    = new Join(_server);
    _commands["PRIVMSG"] = new Privmsg(_server);
    _commands["KICK"]    = new Kick(_server);
    _commands["INVITE"]  = new Invite(_server);
    _commands["TOPIC"]   = new Topic(_server);
    _commands["MODE"]    = new Mode(_server);
    _commands["QUIT"]    = new Quit(_server);
    _commands["PING"]    = new Ping(_server);
    _commands["PART"]    = new Part(_server);

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

        // Chiamata polimorfica: il comando giusto viene scelto a runtime,
        // proprio come Bureaucrat chiamava form.execute() senza sapere
        // quale Form fosse.
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
