/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:06 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 17:25:24 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

Config Server::_config = Config();

Server::Server(): _server_index(0) {}

Server::~Server() {}

Server::Server(const Server &copy) :_port(copy._port),  _server_index(copy._server_index) {}

Server::Server(std::vector<t_server>& data, size_t &index) : _server_index(index){
	_config = Config(data);
}

void Server::setup_socket()
{
	//Creates the socket
	if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		throw ("socket failed");
	//Attaches the socket (optional?)
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &_opt, sizeof(_opt)))
		throw ("setsockopt");
	//Binds the socket
	_address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_port);
    if (bind(_server_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0)
        throw ("bind failed");
}

void Server::start_listen()
{
	if (listen(_server_fd, 3) < 0)
		throw ("listen");
	std::cout << "Start listenting to port: " << _port << std::endl;
}

void Server::start(std::vector<pollfd> &_fds)
{
    setup_socket();
    start_listen();

    pollfd server;
    server.fd = _server_fd;
    server.events = POLLIN;
    _fds.push_back(server);
}

void Server::accept_new_connection(std::vector<pollfd> &_fds)
{
	pollfd client;
	
	std::cout << "Received a new connection" << std::endl;
	if ((client.fd = accept(_server_fd, (struct sockaddr *)&_address, (socklen_t*)&_addrlen)) < 0)
		return (perror("accept"));
	client.events = POLLIN;
	_fds.push_back(client);
	_clients.push_back(client.fd);
}

void Server::remove_client(int fd)
{
	for (auto it = _clients.begin(); it != _clients.end(); it++)
		if (it->get_socket_fd() == fd)
		{
			_clients.erase(it);
			break ;
		}
}

Client *Server::get_client(int fd)
{
	for (Client &client : _clients)
		if (client.get_socket_fd() == fd)
			return (&client);
	return (NULL);
}

int Server::get_port()
{
	return _port;
}

int Server::get_fd()
{
	return _server_fd;
}

void Server::set_port(int port)
{
	_port = port;
}

void Server::set_fd(int fd)
{
	_server_fd = fd;
}