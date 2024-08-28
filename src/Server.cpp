/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:06 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/28 10:28:53 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

//Config Server::_config = Config();

//Server::Server(): index(0) {}

Server::~Server() {}

Server::Server(const Server &copy):
	_port(copy._port), _ip(copy._ip), _name(copy._name),
	_virthost_list(copy._virthost_list),
	_virthost_map(copy._virthost_map),
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
	if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
		throw ("socket failed");
	//Attaches the socket (optional?)
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &_opt, sizeof(_opt))) 
		throw ("setsockopt");
	//Binds the socket
	_address.sin_family = AF_INET;
    _address.sin_addr.s_addr = _ip;
    _address.sin_port = htons(_port);
    if (bind(_server_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0)
        throw (_name + " bind failed");

    std::cout << "[INFO] Socket setup complete. Listening on IP: " << inet_ntoa(_address.sin_addr) << " Port: " << ntohs(_address.sin_port) << "\n";
}

void Server::start_listen()
{
	if (listen(_server_fd, 3) < 0)
		throw ("listen");
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
	_clients.push_back(new Client(client.fd, this));
}

void Server::remove_client(int fd)
{
	for (auto it = _clients.begin(); it != _clients.end(); it++)
		if ((*it)->get_socket_fd() == fd)
		{
			_clients.erase(it);
			break ;
		}
}

Client *Server::get_client(int fd)
{
	for (Client *client : _clients)
		if (client->get_socket_fd() == fd)
			return (client);
	return (NULL);
}

int Server::get_port() const
{
	return _port;
}

std::string Server::get_ip_string() const
{
	struct in_addr ip;
    ip.s_addr = _ip;
	const char * str = inet_ntoa(ip); //no need to clean? stored in static buffer of funciton
	return std::string(str);
}

in_addr_t	Server::get_ip() const
{
	return _ip;
}

int Server::get_fd() const
{
	return _server_fd;
}

std::string Server::get_name()
{
	return _name;
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

// "0.0.0.0" is string for INADDR_ANY
void Server::set_all_config(){
	_port = config.getFirst("main", "listen", DEFAULT_PORT);
	_ip = inet_addr(config.getFirst("main", "host", DEFAULT_IP).c_str());
	_name = config.getFirst("main", "server_name", DEFAULT_SRV_NAME + std::to_string(index));
}

bool Server::checkCookieExist(size_t value)
{
	for (size_t i = 0; i < cookies.size(); i++)
		if (cookies[i] == value)
			return (true);
	return (false);
}

void Server::setNewCookie(size_t value)
{
	static size_t key = 0;
	if (!checkCookieExist(value))
		cookies[key++] = value;
}

void Server::saveCookieInfo(std::string& cookie)
{
	size_t value = (size_t) atoi(cookie.c_str() + 11);
	setNewCookie(value);
}


void Server::setVirthostList(std::vector<size_t> list)
{
	if (list.empty())
		return ;
	_virthost_list = list;
}

void Server::setVirthostMap()
{
	if (_virthost_list.empty())
		return ;
	for (size_t & i :_virthost_list)
	{
		std::string name = config.getAll(i, "main", "server_name", 0);
		if (name.empty())
		{
			std::cout << "[WARRNING] Server " << index
				<< ": virtual server " << i << " has empty name and will be ignored.\n";
			continue;
		}
		if (name == _name || _virthost_map.find(name) != _virthost_map.end())
			std::cout << "[WARRNING] Server " << index
				<< ": virtual server " << i << " is dublicated and will be ignored.\n";
		else
		{
			_virthost_map[name] = i;
			std::cout << "[INFO] Server " << index
				<< ": virtual server " << i 
				<< " is initiated with name " << name << "\n";
		}
	}
}

size_t Server::getVirtHostIndex(std::string host)
{
	std::string srv_name = host.substr(0, host.find(':'));
	if (srv_name == _name)
		return index;
	if (_virthost_map.find(srv_name) == _virthost_map.end())
		return index;
	return _virthost_map[srv_name];
}