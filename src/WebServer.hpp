/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:19 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/08 10:26:42 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <iostream> //std::cout
#include <fstream> //Parse file
#include <sstream> 
#include <string> 

#include <unistd.h> // For close
#include <vector>
#include <poll.h>
#include <algorithm>

#include "Server.hpp"
#include "Parser.hpp"
#include "Config.hpp"


class WebServer
{
	private:
		std::vector<pollfd> _fds;
		std::vector<Server> _servers;
		std::map<size_t, std::vector<size_t>> _real_to_virt;

		void setServers();
		//void parse_file(std::string filename);
		bool fd_is_server(int fd);
		bool fd_is_client(pollfd &pfd);
		//std::vector<size_t>  virtualHostIndices;
		void setRealToVirt();
		std::vector<size_t>  extractVirtualHostsIndices();
		
		WebServer();
		WebServer(const WebServer &copy);
		WebServer &operator=(const WebServer &assignment);
	public:
		~WebServer();
		WebServer(std::vector<t_server>&);
		
		//void setup(std::string filename);
		void setup();
		void run();
		
		const Config config;
};

#endif