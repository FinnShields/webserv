/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 13:36:06 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


Client::Client(int fd) : _fd(fd) {}
Client::Client(const Client &copy) : _fd(copy._fd) {}
Client::~Client() {};
Client &Client::operator=(const Client &assign)
{
	this->_fd = assign._fd;
	return (*this);
}

int Client::get_socket_fd()
{
    return (_fd);
}

static std::string load_index()
{
	std::ifstream file("www/index.html");
	if (!file.is_open())
		return ("HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nError: index.html not found");
	
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	return ("HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + buffer.str());
}

void Client::handle_request(Server srv)
{
    (void) srv;
	
	Request request;
    request.read(_fd);
    request.display();
    
	Response::answer(request);

}

// void Client::handle_request(Server srv)
// {
// 	(void) srv;
    
//     char buffer[MAX_BUFFER_SIZE] = {0};
// 	Request request;
// 	if (recv(_fd, &buffer, MAX_BUFFER_SIZE, 0) < 0)
// 		perror("Recv error");
// 	if (!*buffer)
// 	{
// 		std::cout << "Connection cancelled (empty buffer)" << std::endl;
// 		return ;
// 	}
// 	request.parse(buffer);
// 	request.display();
	
// 	std::string method = request.get("method");
// 	std::string dir = request.get("request-target");
	
// 	std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nYour request: " + method + " " + dir;
// 	if (method == "GET")
// 		if (dir == "/")
// 			response = load_index();
// 	if (send(_fd, response.c_str(), response.size(), 0) < 0)
// 		perror("Send error");
// }

void Client::close_connection(Server &srv)
{
    close(_fd);
    srv.remove_client(_fd);
}