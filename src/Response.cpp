/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:05:15 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 17:56:19 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(int fd, Request &req, Server &srv) : _fd(fd), _req(req), _srv(srv) {}
Response::~Response() {}

void Response::run()
{
	std::string method = _req.get("method");
    
	std::string response = (method == "GET") ? get()
		: (method == "POST") ? post()
		: (method == "DELETE") ? deleteResp()
		: "HTTP/1.1 501 Not Implemented\nContent-Type: text/plain\n\nError: Method not recognized or not implemented";
	
	if (send(_fd, response.c_str(), response.size(), 0) < 0)
		perror("Send error");
}

std::string Response::get()
{	
	std::string method = _req.get("method");
	std::string dir = _req.get("target");
	
	std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nYour request: " + method + " " + dir;
	response.append(" srv: " + _srv.get_name());
	if (dir == "/")
		response = load_index();
	return (response);
}

std::string Response::post()
{
	return ("HTTP/1.1 204 No Content");
}

std::string Response::deleteResp()
{
	return ("HTTP/1.1 204 No Content");
}

std::string Response::load_index()
{
	std::ifstream file("www/index_cgi.html");
	if (!file.is_open())
		return ("HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nError: index.html not found");
	
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	return ("HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + buffer.str());
}
