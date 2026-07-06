#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <set>

class Channel
{
	private:
		std::string		_name;
		std::string		_topic;
		std::string		_key;
		std::set<int>	_members;
		std::set<int>	_operators;
		std::set<int>	_invited;
		bool			_inviteOnly;
		bool			_topicLocked;
		std::size_t		_userLimit;

	public:
		Channel( void );
		Channel( const std::string& name );
		Channel( const Channel& other );
		Channel& operator=( const Channel& other );
		~Channel( void );

		const std::string&		getName( void ) const;
		const std::string&		getTopic( void ) const;
		const std::string&		getKey( void ) const;
		const std::set<int>&	getMembers( void ) const;
		std::size_t				getUserLimit( void ) const;

		void	setTopic( const std::string& topic );
		void	setKey( const std::string& key );
		void	setInviteOnly( bool on );
		void	setTopicLocked( bool on );
		void	setUserLimit( std::size_t limit );

		bool	isInviteOnly( void ) const;
		bool	isTopicLocked( void ) const;
		bool	isFull( void ) const;
		bool	isEmpty( void ) const;

		void	addMember( int fd );
		void	removeMember( int fd );
		bool	isMember( int fd ) const;

		void	addOperator( int fd );
		void	removeOperator( int fd );
		bool	isOperator( int fd ) const;

		void	invite( int fd );
		bool	isInvited( int fd ) const;

		std::string	modeString( void ) const;
};

#endif
