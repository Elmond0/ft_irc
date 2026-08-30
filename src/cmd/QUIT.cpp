#include "../../inc/QUIT.hpp"
#include "../../inc/CommandUtils.hpp"
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <vector>

QUIT::QUIT(Server &server) : ACommand(server) {}

QUIT::~QUIT(void) {}

void QUIT::execute(Client &client, const IrcMessage &msg) {
	std::string reason = "";
    
    if (!msg.trailing.empty())
        reason = msg.trailing;
    else if (!msg.params.empty())
        reason = msg.params[0];
	else
		reason = "Client quit";
	_server.disconnectClient(client, reason);
}
