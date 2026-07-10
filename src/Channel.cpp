/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: giomastr <giomastr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/07 15:50:45 by giomastr          #+#    #+#             */
/*   Updated: 2026/07/07 15:50:52 by giomastr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "Channel.hpp"
#include "Client.hpp"
#include <algorithm>

Channel::Channel()
	: _userLimit(0), _hasUserLimit(false), _inviteOnly(false), _topicRestricted(false)
{
}

Channel::Channel(const std::string &name)
	: _name(name), _userLimit(0), _hasUserLimit(false),
	  _inviteOnly(false), _topicRestricted(false)
{
}

Channel::Channel(const Channel &other)
{
	*this = other;
}

Channel &Channel::operator=(const Channel &other)
{
	if (this != &other)
	{
		_name = other._name;
		_topic = other._topic;
		_key = other._key;
		_userLimit = other._userLimit;
		_hasUserLimit = other._hasUserLimit;
		_inviteOnly = other._inviteOnly;
		_topicRestricted = other._topicRestricted;
		_clients = other._clients;
		_operators = other._operators;
		_invited = other._invited;
	}
	return (*this);
}

Channel::~Channel()
{
}

bool Channel::contains(const std::vector<Client *> &v, Client *c)
{
	for (size_t i = 0; i < v.size(); ++i)
	{
		if (v[i] == c)
			return (true);
	}
	return (false);
}

void Channel::remove(std::vector<Client *> &v, Client *c)
{
	for (size_t i = 0; i < v.size(); ++i)
	{
		if (v[i] == c)
		{
			v.erase(v.begin() + i);
			return ;
		}
	}
}

const std::string &Channel::getName() const
{
	return (_name);
}

const std::string &Channel::getTopic() const
{
	return (_topic);
}

void Channel::setTopic(const std::string &topic)
{
	_topic = topic;
}

void Channel::addClient(Client *client)
{
	if (client && !contains(_clients, client))
		_clients.push_back(client);
}

void Channel::removeClient(Client *client)
{
	remove(_clients, client);
	remove(_operators, client);
	remove(_invited, client);
}

bool Channel::hasClient(Client *client) const
{
	return (contains(_clients, client));
}

bool Channel::isEmpty() const
{
	return (_clients.empty());
}

const std::vector<Client *> &Channel::getClients() const
{
	return (_clients);
}

void Channel::addOperator(Client *client)
{
	if (client && contains(_clients, client) && !contains(_operators, client))
		_operators.push_back(client);
}

void Channel::removeOperator(Client *client)
{
	remove(_operators, client);
}

bool Channel::isOperator(Client *client) const
{
	return (contains(_operators, client));
}

void Channel::addInvited(Client *client)
{
	if (client && !contains(_invited, client))
		_invited.push_back(client);
}

void Channel::removeInvited(Client *client)
{
	remove(_invited, client);
}

bool Channel::isInvited(Client *client) const
{
	return (contains(_invited, client));
}

bool Channel::isInviteOnly() const
{
	return (_inviteOnly);
}

void Channel::setInviteOnly(bool value)
{
	_inviteOnly = value;
}

bool Channel::isTopicRestricted() const
{
	return (_topicRestricted);
}

void Channel::setTopicRestricted(bool value)
{
	_topicRestricted = value;
}

bool Channel::hasKey() const
{
	return (!_key.empty());
}

const std::string &Channel::getKey() const
{
	return (_key);
}

void Channel::setKey(const std::string &key)
{
	_key = key;
}

void Channel::clearKey()
{
	_key.clear();
}

size_t Channel::getUserLimit() const
{
	return (_userLimit);
}

void Channel::setUserLimit(size_t limit)
{
	_userLimit = limit;
	_hasUserLimit = true;
}

void Channel::clearUserLimit()
{
	_userLimit = 0;
	_hasUserLimit = false;
}

std::string Channel::getModeString() const
{
	std::string modes = "+";

	if (_inviteOnly)
		modes += "i";
	if (_topicRestricted)
		modes += "t";
	if (hasKey())
		modes += "k";
	if (_hasUserLimit)
		modes += "l";
	if (modes == "+")
		return ("");
	return (modes);
}

void Channel::broadcast(const std::string &message, Client *exclude) const
{
	for (size_t i = 0; i < _clients.size(); ++i)
	{
		if (_clients[i] != exclude)
			_clients[i]->queueMessage(message);
	}
}
