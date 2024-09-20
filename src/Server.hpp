/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:10 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/19 13:14:01 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <map>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <poll.h>
#include <arpa/inet.h>

#include "Client.hpp"
#include "Parser.hpp"
#include "Config.hpp"

#define DEFAULT_PORT 8080
#define DEFAULT_IP "0.0.0.0"
#define DEFAULT_SRV_NAME "srv-"
#define DEFAULT_METHOD {"GET", "POST", "DELETE", "HEAD", "PUT"}
#define DEFAULT_ALLOWED_METHOD {}

class Client; 

class Server 
{
    private:
        std::vector<std::unique_ptr<Client>> _clients;
        std::unordered_map<size_t, size_t> cookies;
		
        struct sockaddr_in _address;
		int _opt = 1;
		int _addrlen = sizeof(_address);
		int _server_fd;
		int _port;
        in_addr_t _ip;
        std::string _name;
		std::vector<pollfd> *_fds;

        std::vector<size_t> _virthost_list;
        std::map<std::string, size_t> _virthost_map;
        
        
        void setup_socket();
		void start_listen();
        in_addr_t ft_inet_addr(std::string ip_addr);
        std::string ft_inet_ntoa(in_addr_t addr) const;
        
        Server();
    public:
        Server (const Server &copy);
        Server &operator=(const Server &copy);
        Server(std::vector<t_server>&, size_t);
        ~Server();
  
        void start(std::vector<pollfd> &_fds);
        void accept_new_connection(std::vector<pollfd> &_fds);
        void remove_client(int fd);
        Client *get_client(int fd);
        int get_port() const;
        std::string get_ip_string() const;
        in_addr_t   get_ip() const;
        int get_fd() const;
        std::string get_name();
        void set_port(const int &port);
        void set_fd(const int &fd);
        void set_ip(const int &ip);
        void set_name(const std::string &);
        void set_all_config ();
        bool checkCookieExist(size_t value);
        void setNewCookie(size_t value);
        void saveCookieInfo(std::string& cookie);
        void setVirthostList( std::vector<size_t>);
        void setVirthostMap();
        size_t getVirtHostIndex(std::string);
		std::vector<pollfd> *get_fds();

        const size_t index;
        const Config config;
};

#endif
