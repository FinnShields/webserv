/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:36 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/19 15:17:14 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <sys/stat.h>
#include <string>
#include <fstream>
#include <sstream> 
#include <unistd.h>
#include <ctime>
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
		size_t _totalBytesSent;
		//void clean_socket_fd();
		time_t _starttime;
		

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
		pollfd &getCGIwritepollfd();
		int readFromCGI();
		int writeToCgi();
		bool timeout(unsigned int seconds);
		bool isRequestComplete();
};

#endif