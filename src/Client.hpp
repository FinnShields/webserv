/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:36 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/07 18:35:00 by bsyvasal         ###   ########.fr       */
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
#include "Response.hpp"
#include "Request.hpp"
#include "Cgi.hpp"

#define MAX_BUFFER_SIZE 10000

class Server;
class Request;
//class Cgi;

class Client
{
    private:
        int _fd;
        Request *_request;
        std::string _response;
        std::string fileName;

        Client();
    public:
        Client(int socket_fd);
        Client(const Client &copy);
        Client &operator=(const Client &assign);
        ~Client();
        
        int handle_request(Server& srv);
        int send_response();
        void close_connection(Server &srv);
        int get_socket_fd();
        std::string& get_fileName();
};

#endif