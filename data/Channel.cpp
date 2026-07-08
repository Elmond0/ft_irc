/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: giomastr <giomastr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/05 18:47:44 by giomastr          #+#    #+#             */
/*   Updated: 2026/07/06 15:50:48 by giomastr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
#include "Client.hpp"

//OCF

Channel::Channel()	: _userLimit(0), _hasUserLimit(false), _inviteOnly(false), _topicRestricted(false) {}

//Channel::Channel(const std::string &name) : _name(name), _userLimit(0), _hasUserLimit(false), _inviteOnly(false), _topicRestricted(false){}

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
		//_key = other._key;
		_userLimit = other._userLimit;
		_hasUserLimit = other._hasUserLimit;
		_inviteOnly = other._inviteOnly;
		_topicRestricted = other._topicRestricted;
		//_clients = other._clients;
		//_operators = other._operators;
		//_invited = other._invited;
	}
	return (*this);
}

Channel::~Channel() {}

//

bool Channel::isEmpty() const
{
	return (_clients.empty());
}

bool Channel::isTopicRestricted() const
{
	return (_topicRestricted);
}

void Channel::setTopicRestricted(bool value) {_topicRestricted = value;}
