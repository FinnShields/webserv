/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:05:15 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 14:15:57 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(int fd, Request &req) : _fd(fd), _req(req) {}
Response::~Response() {};

void Response::get()
{	
	std::string method = _req.get("method");
	std::string dir = _req.get("request-target");
	
	std::string respons = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nYour request: " + method + " " + dir;
	if (dir == "/")
		respons = load_index();
	if (send(_fd, respons.c_str(), respons.size(), 0) < 0)
		perror("Send error");
}

void Response::post()
{
	std::cout << "POST method is not supported" << std::endl;
}
void Response::run()
{
	std::string method = _req.get("method");
    if (method == "GET")
        get();
    else if (method == "POST")
        post();
}

std::string Response::load_index()
{
	std::ifstream file("www/index.html");
	if (!file.is_open())
		return ("HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nError: index.html not found");
	
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	return ("HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + buffer.str());
}
