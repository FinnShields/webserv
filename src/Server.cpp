/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:06 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/22 12:51:31 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

//Config Server::_config = Config();

//Server::Server(): index(0) {}

Server::~Server() {}

Server::Server(const Server &copy):
	_port(copy._port), _ip(copy._ip), _name(copy._name), 
	fileName(copy.fileName), allowedMethods(copy.allowedMethods),
	index(copy.index), 
	config(copy.config){}

Server::Server(std::vector<t_server>& data, size_t ind): 
	index(ind),
	config(Config(data, ind))
{
	set_all_config();
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
    _address.sin_addr.s_addr = _ip;
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

int Server::get_port() const
{
	return _port;
}

int Server::get_fd() const
{
	return _server_fd;
}

std::string Server::getAllowedMethods() const
{
	return (allowedMethods);
}

std::string Server::get_name()
{
	return _name;
}

std::string Server::getFileName()
{
	return fileName;
}

void Server::setFileName(std::string& fileName)
{
	this->fileName = fileName;
}

void Server::clearFileName()
{
	this->fileName.clear();
}

void Server::set_port(const int &port)
{
	_port = port;
}

void Server::set_fd(const int &fd)
{
	_server_fd = fd;
}

void Server::set_ip(const int &ip)
{
	_ip = ip;
}

void Server::set_name(const std::string &name)
{
	_name = name;
}

//"0.0.0.0" is string for INADDR_ANY
void Server::set_all_config(){
	_port = config.getFirst("main", "listen", DEFAULT_PORT);
	_ip = inet_addr(config.getFirst("main", "host", "0.0.0.0").c_str());
	_name = config.getFirst("main", "server_name", "srv-" + std::to_string(index));
	allowedMethods = config.getFirst("main", "limit_except", "GET POST DELETE");
}