/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:19 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/30 14:37:20 by bsyvasal         ###   ########.fr       */
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

#define SOCKETTIMEOUT 30 //Seconds before a socket times out
#define POLLTIMEOUT 30000 //Miliseconds without actions before checking if some has timed out 

class WebServer
{
	private:
		static int running;
		static void setExit(int signal);
		
		std::vector<pollfd> _fds;
		std::vector<Server> _servers;
		std::map<size_t, std::vector<size_t>> _real_to_virt;
		std::map<int, pollfd *> _cgi_readfd_clients;
		std::map<int, pollfd *> _cgi_writefd_clients;

		bool	fd_is_server(int fd);
		int		fd_is_client(pollfd &pfd);
		int		fd_is_cgi(pollfd pfd);
		int		fd_is_cgiwrite(pollfd &pfd);
		int		fd_is_client_write(pollfd &pfd, Client* client);
		int		fd_is_client_read(pollfd &pfd, Client* client);
		void	addCGItoPollfd(pollfd &pfd, Client *client);
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