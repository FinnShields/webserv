/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:10 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/22 12:33:16 by apimikov         ###   ########.fr       */
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
#include <arpa/inet.h>

#include "Client.hpp"
#include "Parser.hpp"
#include "Config.hpp"

#define DEFAULT_PORT 8080

class Client; 

class Server 
{
    private:
        std::vector<Client> _clients;
		
        struct sockaddr_in _address;
		int _opt = 1;
		int _addrlen = sizeof(_address);
		int _server_fd;
		int _port;
        in_addr_t _ip;
        std::string _name;
        
        void setup_socket();
		void start_listen();
        
        Server();
        Server &operator=(const Server &copy);
    public:
        Server (const Server &copy);
        Server(std::vector<t_server>&, size_t);
        ~Server();
  
        void start(std::vector<pollfd> &_fds);
        void accept_new_connection(std::vector<pollfd> &_fds);
        void remove_client(int fd);
        Client *get_client(int fd);
        int get_port() const;
        int get_fd() const;
        std::string get_name();
        void set_port(const int &port);
        void set_fd(const int &fd);
        void set_ip(const int &ip);
        void set_name(const std::string &);
        void set_all_config ();

        const size_t index;
        const Config config;
};

#endif