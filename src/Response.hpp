/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:06:10 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 17:53:48 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include "Request.hpp"
#include "Server.hpp"

class Request;

class Response
{
	private:
		int	_fd;
		Request &_req;
		Server &_srv;
		std::string get();
		std::string post();
		std::string load_index();

		Response();
		Response(const Response &copy);
		Response &operator=(const Response &assign);
	public:
		Response(int fd, Request &req, Server &srv);
		~Response();
		
		void run();
};
#endif
