/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:36 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/26 14:35:36 by bsyvasal         ###   ########.fr       */
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
        int		_fd;
        Server	*_server;
        std::unique_ptr<Request>	_request;
        std::unique_ptr<Response>	_res;
        std::string	_response;
		size_t	_totalBytesSent;
		time_t	_starttime;
		pollfd	*_cgireadpfd;
        bool	_responseSent;
		bool	_force_closeconnection;
		int		_resets;
        int		_sessionID;
		int		send_cgi_response();
		
        Client();
        Client(const Client &copy);
        Client &operator=(const Client &assign);
    public:
        Client(int socket_fd, Server *srv);
        ~Client();
        
		bool timeout(unsigned int seconds);
		bool isRequestComplete();
		void setcgiireadpfd(pollfd *pfd);
        void close_connection();
		void resetForNextRequest();
        void setSessionID(int id);
        int	handle_request();
        int send_response();
		int readFromCGI();
		int writeToCgi();
		int shouldCloseConnection();
        int getSessionID();
        int get_socket_fd();
		int get_cgi_fd();
		int getCGIwritefd();
};

#endif