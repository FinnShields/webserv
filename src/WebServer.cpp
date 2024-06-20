/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:14 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 12:22:17 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "WebServer.hpp"

WebServer::WebServer() {}

WebServer::~WebServer() {}

void WebServer::parse_file(std::string filename)
{
    std::ifstream config_file(filename);
    std::string line;
    if (config_file.is_open())
	{
		Server srv;
		std::cout << "Reading file" << std::endl;
        while (getline(config_file, line))
		{
            if (line.find("port") != std::string::npos)
                srv.set_port(std::stoi(line.substr(line.find("=") + 1)));
        }
		srv.set_ip(INADDR_ANY);
		_servers.push_back(srv);
        config_file.close();
		return ; 
    }
	std::cout << "No cfg file, using default" << std::endl;
	Server srv;
	Server srv1;
	srv.set_port(DEFAULT_PORT);
	srv1.set_port(1111);
	srv.set_ip(INADDR_ANY);
	srv1.set_ip(inet_addr("127.0.0.2"));
	srv.set_name("srv");
	srv1.set_name("srv1");
	_servers.push_back(srv1);
	_servers.push_back(srv);
}


void WebServer::setup(std::string filename)
{
	try
	{
		parse_file(filename);
		for (Server &srv : _servers)
			srv.start(_fds);
	}
	catch(char const *e)
	{
		perror(e);
		exit(EXIT_FAILURE);
	}
	
}

bool WebServer::fd_is_server(int fd)
{
	for (Server &srv : _servers)
		if (fd == srv.get_fd())
		{
			srv.accept_new_connection(_fds);
			return (true);
		}
	return (false);
}

void WebServer::fd_is_client(int fd)
{
	Client *client;

	for (Server &srv : _servers)
		if ((client = srv.get_client(fd)))
		{
			client->handle_request(srv);
			client->close_connection(srv);
			return ;
		}
}

void WebServer::run()
{
	while (1)
	{
		std::cout << "\nWaiting for action... - size of pollfd vector: " << _fds.size() << std::endl;
		int poll_result = poll(_fds.data(), _fds.size(), -1);
		if (poll_result == -1)
			return (perror("poll"));
		for (std::vector<pollfd>::iterator pfdit = _fds.begin(); pfdit != _fds.end();)
		{
			if (pfdit->revents & POLLIN)
			{
				if (fd_is_server(pfdit->fd))
					break;
				fd_is_client(pfdit->fd);
				_fds.erase(pfdit);
				break;
			}
			pfdit++;
		}
    }
}
