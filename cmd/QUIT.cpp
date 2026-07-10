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
        if (!chan.isMember(client.getFd()))
            continue;

        const std::set<int>& members = chan.getMembers();
        for (std::set<int>::const_iterator m = members.begin();
             m != members.end(); ++m)
        {
            if (*m != client.getFd() && notified.insert(*m).second)
                server.sendToClient(*m, line);
        }

        chan.removeMember(client.getFd());
        if (chan.isEmpty())
            channels.erase(current);
    }

    server.sendToClient(client.getFd(), "ERROR :Closing Link\r\n");
    int fd = client.getFd();
    close(fd);
    server.getClients().erase(fd);
}
