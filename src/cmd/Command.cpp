#include "../../inc/Commands.hpp"
#include <sstream>

Command::Command(Server& server) : _server(server) {}

Command::Command(const Command& other) : _server(other._server) {}

Command::~Command(void) {}

Command::NumericError::NumericError(int code, const std::string& text)
	: _code(code), _text(text) {}

Command::NumericError::~NumericError() throw() {}

int Command::NumericError::code() const { return _code; }

const std::string& Command::NumericError::text() const { return _text; }

const char* Command::NumericError::what() const throw() { return _text.c_str(); }


void Command::numeric(Client& client, int code, const std::string& text)
{
	std::ostringstream oss;
	oss << ":" << SERVER_NAME << " " << code << " " << nickOrStar(client)
		<< " " << text << "\r\n";
	_server.sendToClient(client.getFd(), oss.str());
}
