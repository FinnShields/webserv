/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:19 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/12 03:52:26 by bsyvasal         ###   ########.fr       */
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
#include "Response.hpp"


class WebServer
{
	private:
		static int running;
		std::vector<pollfd> _fds;
		std::vector<std::unique_ptr<Server>> _servers;
		std::map<size_t, std::vector<size_t>> _real_to_virt;
		std::map<int, pollfd *> _cgi_clients;


		void setServers();
		static void closeAllThenExit(int signal);
		//void parse_file(std::string filename);
		bool fd_is_server(int fd);
		int fd_is_client(pollfd &pfd);
		bool fd_is_cgi(int fd);
		//std::vector<size_t>  virtualHostIndices;
		void setRealToVirt();
		std::vector<size_t>  extractVirtualHostsIndices();
		void setCgiToPoll(pollfd &, Client *);
		std::vector<pollfd>::iterator shutdown(std::vector<pollfd>::iterator &it);
		void removeAllSocketsAndCGI();
		
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