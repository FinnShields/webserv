/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:19 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 12:22:23 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <iostream> //std::cout
#include <fstream> //Parse file
#include <sstream> 

#include <unistd.h> // For close
#include <vector>
#include <poll.h>
#include <algorithm>

#include "Server.hpp"

#define DEFAULT_PORT 8080

class WebServer
{
	private:
		std::vector<pollfd> _fds;
		std::vector<Server> _servers;

		void parse_file(std::string filename);
		bool fd_is_server(int fd);
		void fd_is_client(int fd);
		
		WebServer(const WebServer &copy);
		WebServer &operator=(const WebServer &assignment);
	public:
		WebServer();
		~WebServer();
		
		void setup(std::string filename);
		void run();
};

#endif