/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:19 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/19 23:05:31 by bsyvasal         ###   ########.fr       */
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

#define SOCKETTIMEOUT 180
#define POLLTIMEOUT 10000



class WebServer
{
	private:
		static int running;
		static void setExit(int signal);
		
		std::vector<pollfd> _fds;
		std::vector<Server> _servers;
		std::map<size_t, std::vector<size_t>> _real_to_virt;
		std::map<int, pollfd *> _cgi_readfd_clients;
		std::map<int, pollfd *> _cgi__writefd_clients;

		void setServers();
		//void parse_file(std::string filename);
		bool fd_is_server(int fd);
		int fd_is_client(pollfd &pfd);
		int fd_is_cgi(pollfd pfd);
		int fd_is_cgiwrite(pollfd &pfd);
		//std::vector<size_t>  virtualHostIndices;
		void setRealToVirt();
		std::vector<size_t>  extractVirtualHostsIndices();
		bool checkTimer(int timeout_seconds);
		bool eraseAndContinue(std::vector<pollfd>::iterator &it, std::string from);
		void iterateAndRunActiveFD();
		void cleanexit();
		
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