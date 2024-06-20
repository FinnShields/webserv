/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:10 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 12:22:12 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <map>
#include <vector>
#include <iostream>
#include <poll.h>

#include "Client.hpp"
#include "Parser.hpp"
#include "Config.hpp"

typedef std::map<std::string, std::string> config;

class Client; 

class Server 
{
    private:
        
        
        std::vector<Client> _clients;
        std::map<std::string, config> _locations;
		
        struct sockaddr_in _address;
		int _opt = 1;
		int _addrlen = sizeof(_address);
		int _server_fd;
		int _port;
        
        void setup_socket();
		void start_listen();
        
        Server &operator=(const Server &copy);
    public:
        Server();
        Server (const Server &copy);
        Server(std::vector<t_server>&, size_t&);
        //Server (const Config&);
        ~Server();
  
        void start(std::vector<pollfd> &_fds);
        void accept_new_connection(std::vector<pollfd> &_fds);
        void remove_client(int fd);
        Client *get_client(int fd);
        int get_port();
        int get_fd();
        void set_port(int port);
        void set_fd(int fd);
        static Config _config;
        const size_t _server_index;
};

#endif