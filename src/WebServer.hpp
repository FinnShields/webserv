/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:19 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/26 15:19:54 by bsyvasal         ###   ########.fr       */
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

		bool	fd_is_server(int fd);
		int		fd_is_client(pollfd &pfd);
		int		fd_is_cgi(pollfd pfd);
		int		fd_is_cgiwrite(pollfd &pfd);
		void	setRealToVirt();
		std::vector<size_t>  extractVirtualHostsIndices();
		bool	checkTimer(int timeout_seconds);
		void	iterateAndRunActiveFD();
		int		iterateAndCheckIncomingConnections();
		void	cleanexit();
		
		WebServer();
		WebServer(const WebServer &copy);
		WebServer &operator=(const WebServer &assignment);
	public:
		const Config config;
		
		~WebServer();
		WebServer(std::vector<t_server>&);
		
		void	setup();
		void	run();
};
#endif