/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:14 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/03 06:11:26 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "Config.hpp"
#include "WebServer.hpp"

WebServer::~WebServer() {}

WebServer::WebServer(std::vector<t_server>& data):config(Config(data, 0)) {}

void WebServer::setRealToVirt()
{
	_real_to_virt = config.realToVirtualHosts();
}

std::vector<size_t> WebServer::extractVirtualHostsIndices()
{
	std::vector<size_t> indices;
	for (const auto& [key, value] : _real_to_virt)
	{
		indices.insert(indices.end(), value.begin(), value.end());
	}
	return indices;
}

void WebServer::setup()
{
	setRealToVirt();
	std::vector<size_t> indices = extractVirtualHostsIndices();
	try  // try to be removed as it exist in main 
	{
		std::cout << "[INFO] Total number of servers: " << config.size()
			<< " . Total number of virtual hosts " << indices.size()
			<< ".\n";
		for (size_t i = 0; i < config.size(); ++i)
		{
			auto it = std::find(indices.begin(), indices.end(), i);
			if (it != indices.end())
				continue;
			_servers.emplace_back(config.getAll(), i);
			std::cout << "[INFO] Server " << i << " is created.\n";
			_servers.back().setVirthostList(_real_to_virt[i]);
			_servers.back().setVirthostMap();
			std::cout << "index=" << _servers.back().index << "\n";
			//std::string nnn ="virt";
			//std::cout << _servers.back().getVirtHostIndex(nnn);
		}
		for (Server &srv : _servers)
		{
			srv.start(_fds);
			std::cout << "[INFO] Server " << srv.index << " is started with port " << srv.get_port() << "\n";
		}
	}
	catch(char const *e)
	{
		perror(e);
		std::exit(EXIT_FAILURE);
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
