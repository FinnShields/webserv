/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:06:10 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/26 13:55:17 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <dirent.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include "Request.hpp"
#include "Server.hpp"

#define RESPONSE_501 "HTTP/1.1 501 Not Implemented\nContent-Type: text/plain\n\nError: Method not recognized or not implemented"
#define RESPONSE_500 "HTTP/1.1 500 Internal Server Error\nContent-Type: text/plain\n"
#define RESPONSE_405 "HTTP/1.1 405 Method Not Allowed\nContent-Type: text/plain\n"
#define STATUS_LINE_200 "HTTP/1.1 200 OK\n" 



class Request;
class Server;

class Response
{
	private:
		int	_fd;
		Request &_req;
		Server &_srv;
		std::string get(Server& _srv);
		std::string post(Server& _srv);
		std::string deleteResp(Server& _srv);
		std::string load_index();
		std::string load_directory_listing();
		int saveFile();
        int deleteFile(std::string &);
		void replacePercent20withSpace(std::string &str);
		bool checkFileType(std::string& fileName);

		Response();
		Response(const Response &copy);
		Response &operator=(const Response &assign);
	public:
		Response(int fd, Request &req, Server &srv);
		~Response();
		
		void run();
};
#endif
