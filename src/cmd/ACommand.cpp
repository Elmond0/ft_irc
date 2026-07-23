#include "../../inc/ACommand.hpp"
#include "../../inc/CommandUtils.hpp"
#include <sstream>

ACommand::ACommand(Server &server) : _server(server) {}

ACommand::~ACommand(void) {}

ACommand::NumericError::NumericError(int code, const std::string &text)
    : _code(code), _text(text) {}

ACommand::NumericError::~NumericError() throw() {}

int ACommand::NumericError::code() const { return _code; }

const std::string &ACommand::NumericError::text() const { return _text; }

const char *ACommand::NumericError::what() const throw() { return _text.c_str(); }

void ACommand::numeric(Client &client, int code, const std::string &text)
{
    std::ostringstream oss;
    oss << ":" << SERVER_NAME << " " << code << " " << nickOrStar(client)
        << " " << text << "\r\n";
    _server.sendToClient(client.getFd(), oss.str());
}
