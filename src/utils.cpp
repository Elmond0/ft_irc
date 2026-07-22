#include "../inc/Commands.hpp"

static char ircLower(char c) {
  if (c >= 'A' && c <= 'Z')
    return c + 32;
  switch (c) {
    case '[':  return '{';
    case ']':  return '}';
    case '\\': return '|';
  }
  return c;
}

bool sameNick(const std::string &a, const std::string &b)
{
  if (a.size() != b.size())
    return false;
  for (std::size_t i = 0; i < a.size(); ++i) {
    if (ircLower(a[i]) != ircLower(b[i]))
      return false;
  }
  return true;
}

std::string nickOrStar(const Client &client)
{
  if (client.getNickname().empty())
    return "*";
  return client.getNickname();
}

std::string userPrefix(const Client &client)
{
  return std::string(":") + client.getPrefix();
}

Client *findClientByNick(Server &server, const std::string &nick)
{
  std::map<int, Client> &clients = server.getClients();

  for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) 
  {
    if (sameNick(it->second.getNickname(), nick))
      return &it->second;
  }
  return NULL;
}

Channel *findChannel(Server &server, const std::string &name) 
{
  std::map<std::string, Channel> &channels = server.getChannels();
  std::map<std::string, Channel>::iterator it = channels.find(name);

  if (it == channels.end())
    return NULL;
  return &it->second;
}

void broadcastToChannel(Server &server, Channel &chan, const std::string &line, int exceptFd)
{
  const std::vector<Client *> &members = chan.getClients();

  for (std::size_t i = 0; i < members.size(); ++i) 
  {
    if (members[i]->getFd() != exceptFd)
      server.sendToClient(members[i]->getFd(), line);
  }
}

std::vector<std::string> splitComma(const std::string &s)
{
  std::vector<std::string> out;
  std::size_t start = 0;

  while (start <= s.size())
  {
    std::size_t comma = s.find(',', start);
    if (comma == std::string::npos) 
	{
      out.push_back(s.substr(start));
      break;
    }
    out.push_back(s.substr(start, comma - start));
    start = comma + 1;
  }
  return out;
}

void rejectRegistration(Client &client, Server &server, int code, const std::string &text)
{
  std::ostringstream oss;

  oss << ":" << SERVER_NAME << " " << code << " " << nickOrStar(client) << " " << text << "\r\n";
  server.sendToClient(client.getFd(), oss.str());
  server.sendToClient(client.getFd(), std::string("ERROR :Closing Link: ") + text + "\r\n");

  int fd = client.getFd();
  close(fd);
  server.getClients().erase(fd);
}

void finishRegistrationAttempt(Client &client, Server &server) 
{
  if (client.getNickname().empty() || client.getUsername().empty())
    return;
  if (client.isRegistered())
    sendWelcome(client, server);
  else
    rejectRegistration(client, server, 464, ":Password incorrect");
}

void sendWelcome(Client &client, Server &server) 
{
  std::string nick = client.getNickname();

  server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME + " 001 " + nick + " :Welcome to the IRC Network " + client.getPrefix() + "\r\n");
  server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME + " 002 " + nick + " :Your host is " + SERVER_NAME + ", running version 1.0\r\n");
  server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME + " 003 " + nick + " :This server was created today\r\n");
  server.sendToClient(client.getFd(), std::string(":") + SERVER_NAME + " 004 " + nick + " " + SERVER_NAME + " 1.0 o o\r\n");
}
