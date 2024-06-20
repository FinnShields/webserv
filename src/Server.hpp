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
#include <arpa/inet.h>


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
        in_addr_t _ip;
        std::string _name;
        
        void setup_socket();
		void start_listen();
        
        Server &operator=(const Server &copy);
    public:
        Server();
        Server (const Server &copy);
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

};

#include "Client.hpp"

#endif