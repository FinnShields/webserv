/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:14 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/14 23:25:07 by bsyvasal         ###   ########.fr       */
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

bool WebServer::fd_is_client(pollfd &pfd)
{
	Client *client;
    
	for (Server &srv : _servers)
		if ((client = srv.get_client(pfd.fd)))
		{
            if (pfd.revents & POLLIN)
            {
                std::cout << " pollin" << std::endl;
			    int ret = client->handle_request(srv);
                if (ret == 0)
                    pfd.events = POLLOUT;
                if (ret == -1)
                {
    			    client->close_connection(srv);
                    return true;
                }
                return false;
            }
            if (pfd.revents & POLLOUT)
            {
                std::cout << " pollout" << std::endl;
                client->send_response();
                client->close_connection(srv);
                return true;
            }
		}
	return false;
}

void WebServer::run()
{
	while (1)
	{
		std::cout << "Waiting for action... - size of pollfd vector: " << _fds.size() << std::endl;
		int poll_result = poll(_fds.data(), _fds.size(), -1);
		if (poll_result == -1)
			return (perror("poll"));
		for (std::vector<pollfd>::iterator it = _fds.begin(); it != _fds.end();)
		{
			if (it->revents & (POLLIN|POLLOUT))
			{
				if (fd_is_server(it->fd))
					break;
                if (fd_is_client(*it))
                {
                    std::cout << "erasing it" << std::endl;
                    it = _fds.erase(it);
                    continue;
                }
			}
            it++;
		}
    }
}
