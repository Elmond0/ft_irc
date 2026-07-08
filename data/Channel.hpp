/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: giomastr <giomastr@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/07/05 16:39:58 by giomastr          #+#    #+#             */
/*   Updated: 2026/07/05 18:58:13 by giomastr         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
# define CHANNEL_HPP

# include <string>
# include <vector>

class Channel
{
	private:
		std::string				_name;
		std::string				_topic;
		//std::string				_key;
		size_t					_userLimit;
		bool					_hasUserLimit;
		bool					_inviteOnly;
		bool					_topicRestricted;
		//std::vector<Client *>	_clients;
		//std::vector<Client *>	_operators;
		//std::vector<Client *>	_invited;

		//static bool	contains(const std::vector<Client *> &v, Client *c);
		//static void	remove(std::vector<Client *> &v, Client *c);

	public:
		Channel();
		//explicit Channel(const std::string &name);
		Channel(const Channel &other);
		Channel &operator=(const Channel &other);
		~Channel();

		//const std::string	&getName() const;

		//const std::string	&getTopic() const;
		//void				setTopic(const std::string &topic);

		//void				addClient(Client *client);
		//void				removeClient(Client *client);
		//bool				hasClient(Client *client) const;
		bool				isEmpty() const;
		//const std::vector<Client *>	&getClients() const;

		//void				addOperator(Client *client);
		//void				removeOperator(Client *client);
		//bool				isOperator(Client *client) const;

		//void				addInvited(Client *client);
		//void				removeInvited(Client *client);
		//bool				isInvited(Client *client) const;

		//bool				isInviteOnly() const;
		//void				setInviteOnly(bool value);

		//bool				isTopicRestricted() const;
		//void				setTopicRestricted(bool value);

		//bool				hasKey() const;
		//const std::string	&getKey() const;
		//void				setKey(const std::string &key);
		//void				clearKey();

		//size_t				getUserLimit() const;
		//void				setUserLimit(size_t limit);
		//void				clearUserLimit();

		///* stringa tipo "+itk" per il reply MODE/RPL_CHANNELMODEIS */
		//std::string			getModeString() const;

		///* invia message a tutti i membri, escludendo eventualmente uno */
		//void				broadcast(const std::string &message, Client *exclude = 0) const;


};


#endif