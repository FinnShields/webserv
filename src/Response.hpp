/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:06:10 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 13:59:22 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include "Request.hpp"

class Request;

class Response
{
	private:
		int	_fd;
		Request &_req;
		void get();
		void post();
		std::string load_index();

		Response();
		Response(const Response &copy);
		Response &operator=(const Response &assign);
	public:
		Response(int fd, Request &req);
		~Response();
		
		void run();
};
#endif
