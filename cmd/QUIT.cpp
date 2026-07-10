#include "../inc/Commands.hpp"
#include <set>
#include <unistd.h>

void QUIT(Client& client, const IrcMessage& msg, Server& server)
{
    std::string reason = msg.trailing.empty() ? "Client Quit" : msg.trailing;
    std::string line = userPrefix(client) + " QUIT :" + reason + "\r\n";

    std::set<int> notified;
    std::map<std::string, Channel>& channels = server.getChannels();
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
                server.sendToClient(members[m]->getFd(), line);
        }

        chan.removeClient(&client);
        if (chan.isEmpty())
            channels.erase(current);
    }

    server.sendToClient(client.getFd(), "ERROR :Closing Link\r\n");
    int fd = client.getFd();
    close(fd);
    server.getClients().erase(fd);
}
