/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/26 13:47:28 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


Client::Client(int fd) : _fd(fd) {}
Client::Client(const Client &copy) : _fd(copy._fd), fileName(copy.fileName) {}
Client::~Client() {};
Client &Client::operator=(const Client &assign)
{
	this->_fd = assign._fd;
    this->fileName = assign.fileName;
	return (*this);
}

int Client::get_socket_fd()
{
    return (_fd);
}

std::string& Client::get_fileName()
{
    return (this->fileName);
}

void Client::handle_request(Server& srv)
{
	Request request;
    request.read(_fd);
    request.display();
	Response resp(_fd, request, srv);
	resp.run();
}

void Client::close_connection(Server &srv)
{
    close(_fd);
    srv.remove_client(_fd);
}