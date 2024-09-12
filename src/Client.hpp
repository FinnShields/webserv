/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:36 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/12 12:44:29 by bsyvasal         ###   ########.fr       */
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
class Response;

class Client
{
    private:
        int _fd;
        Server *_server;
        std::unique_ptr<Request> _request;
        std::string _response;
        std::unique_ptr<Response> _res;
        bool _responseSent;
		bool _isCGI;
		void clean_socket_fd();

        Client();
        Client(const Client &copy);
        Client &operator=(const Client &assign);
    public:
        Client(int socket_fd, Server *srv);
        ~Client();
        
        int handle_request();
        int send_response();
        void close_connection();
        int get_socket_fd();
        bool responseReady();
		int get_cgi_fd();
		int readFromCGI();
};

#endif