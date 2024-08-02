/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:14 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/02 06:41:32 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "Config.hpp"
#include "WebServer.hpp"

WebServer::~WebServer() {}

WebServer::WebServer(std::vector<t_server>& data):config(Config(data, 0)) {}

std::vector<size_t> WebServer::extractVirtualHostsIndices()
{
	std::map<size_t, std::vector<size_t>> mapping = config.realToVirtualHosts();
	std::vector<size_t> indices;
	for (const auto& [key, value] : mapping)
	{
		indices.insert(indices.end(), value.begin(), value.end());
	}
	return indices;
}

void WebServer::setup()
{
	std::vector<size_t> indexes = extractVirtualHostsIndices();
	try
	{
		std::cout << "Number of servers: " << config.size() << "\n";
		for (size_t i = 0; i < config.size(); ++i) {
			auto it = std::find(indexes.begin(), indexes.end(), i);
			if (it != indexes.end())
				continue;
			_servers.emplace_back(config.getAll(), i);
			std::cout << "Server " << i << " is initialized with port " << _servers[i].get_port() << "\n";
		}
		for (Server &srv : _servers){
			std::cout << "Starting server " << srv.index << " with port " << srv.get_port() << "\n";
			srv.start(_fds);
		}
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
