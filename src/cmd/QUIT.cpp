#include "../../inc/QUIT.hpp"
#include "../../inc/CommandUtils.hpp"
#include <cstddef>
#include <map>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

QUIT::QUIT(Server &server) : ACommand(server) {}

QUIT::~QUIT(void) {}

void QUIT::execute(Client &client, const IrcMessage &msg)
{
    std::string reason = msg.trailing.empty() ? "Client Quit" : msg.trailing;
    std::string line = userPrefix(client) + " QUIT :" + reason + "\r\n";

    std::set<int> notified;
    std::map<std::string, Channel>& channels = _server.getChannels();
    std::map<std::string, Channel>::iterator it = channels.begin();

    while (it != channels.end())
    {
        std::map<std::string, Channel>::iterator current = it++;
        Channel& chan = current->second;

        if (!chan.hasClient(&client))
            continue;

        const std::vector<Client*>& members = chan.getClients();

        for (std::size_t m = 0; m < members.size(); ++m)
        {
            if (members[m] != &client && notified.insert(members[m]->getFd()).second)
                _server.sendToClient(members[m]->getFd(), line);
        }

        chan.removeClient(&client);
        if (chan.isEmpty())
            channels.erase(current);
    }

    _server.sendToClient(client.getFd(), "ERROR :Closing Link\r\n");
    int fd = client.getFd();
    close(fd);
    _server.getClients().erase(fd);
}
