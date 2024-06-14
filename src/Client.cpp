/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 16:30:28 by bsyvasal         ###   ########.fr       */
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

void Client::handle_request(Server srv)
{
    (void) srv;
	
	Request request;
    request.read(_fd);
    request.display();
    
	Response resp(_fd, request);
	resp.run();
}

void Client::close_connection(Server &srv)
{
    close(_fd);
    srv.remove_client(_fd);
}