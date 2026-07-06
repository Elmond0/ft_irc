#include "Channel.hpp"

Channel::Channel( void ) : _inviteOnly(false), _topicLocked(false), _userLimit(0) {}

Channel::Channel( const std::string& name ) : _name(name), _inviteOnly(false), _topicLocked(false), _userLimit(0) {}

Channel::Channel( const Channel& other ) : _name(other._name), _topic(other._topic), _key(other._key), _members(other._members), _operators(other._operators), _invited(other._invited), _inviteOnly(other._inviteOnly), _topicLocked(other._topicLocked), _userLimit(other._userLimit) {}

Channel& Channel::operator=( const Channel& other ) {
	if (this != &other) {
		_name = other._name;
		_topic = other._topic;
		_key = other._key;
		_members = other._members;
		_operators = other._operators;
		_invited = other._invited;
		_inviteOnly = other._inviteOnly;
		_topicLocked = other._topicLocked;
		_userLimit = other._userLimit;
	}
	return *this;
}

Channel::~Channel( void ) {}

const std::string&	Channel::getName( void ) const { return _name; }

const std::string&	Channel::getTopic( void ) const { return _topic; }

const std::string&	Channel::getKey( void ) const { return _key; }

const std::set<int>&	Channel::getMembers( void ) const { return _members; }

std::size_t	Channel::getUserLimit( void ) const { return _userLimit; }

void	Channel::setTopic( const std::string& topic ) { _topic = topic; }

void	Channel::setKey( const std::string& key ) { _key = key; }

void	Channel::setInviteOnly( bool on ) { _inviteOnly = on; }

void	Channel::setTopicLocked( bool on ) { _topicLocked = on; }

void	Channel::setUserLimit( std::size_t limit ) { _userLimit = limit; }

bool	Channel::isInviteOnly( void ) const { return _inviteOnly; }

bool	Channel::isTopicLocked( void ) const { return _topicLocked; }

bool	Channel::isFull( void ) const { return _userLimit > 0 && _members.size() >= _userLimit; }

bool	Channel::isEmpty( void ) const { return _members.empty(); }

void	Channel::addMember( int fd ) {
	_members.insert(fd);
	_invited.erase(fd);
}

void	Channel::removeMember( int fd ) {
	_members.erase(fd);
	_operators.erase(fd);
	_invited.erase(fd);
}

bool	Channel::isMember( int fd ) const { return _members.count(fd) > 0; }

void	Channel::addOperator( int fd ) { _operators.insert(fd); }

void	Channel::removeOperator( int fd ) { _operators.erase(fd); }

bool	Channel::isOperator( int fd ) const { return _operators.count(fd) > 0; }

void	Channel::invite( int fd ) { _invited.insert(fd); }

bool	Channel::isInvited( int fd ) const { return _invited.count(fd) > 0; }

std::string	Channel::modeString( void ) const {
	std::string modes = "+";
	if (_inviteOnly)
		modes += "i";
	if (_topicLocked)
		modes += "t";
	if (!_key.empty())
		modes += "k";
	if (_userLimit > 0)
		modes += "l";
	return modes;
}
