/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:36 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/28 04:30:50 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/stat.h>
#include <string>
#include <fstream>
#include <sstream> 
#include <unistd.h>
#include "Server.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Cgi.hpp"

#define MAX_BUFFER_SIZE 10000

class Server;
class Request;
//class Cgi;

class Client
{
    private:
        int _fd;
        std::string fileName;

        Client();
    public:
        Client(int socket_fd);
        Client(const Client &copy);
        Client &operator=(const Client &assign);
        ~Client();
        
        void handle_request(Server& srv);
        void close_connection(Server &srv);
        int get_socket_fd();
        std::string& get_fileName();
};

#endif