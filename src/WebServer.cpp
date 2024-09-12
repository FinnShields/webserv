/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:14 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/12 04:39:05 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "Config.hpp"
#include "WebServer.hpp"

static std::vector<pollfd> *_fds_ptr;
int WebServer::running = 1;

WebServer::~WebServer() 
{
	// removeAllSocketsAndCGI();
	_servers.clear();
	_fds.clear();
}

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

void WebServer::closeAllThenExit(int signal)
{
    // std::vector<pollfd> _fds;
    // _fds = *_fds_ptr;
    if (signal == SIGINT)
        std::cout << " -- Closing due to SIGINT (ctl-c) " << std::endl;
    WebServer::running = 0;
    // for (size_t i = 0; i < _fds.size(); i ++)
    //     close(_fds[i].fd);
	// exit(130);
}

void WebServer::setup()
{
    signal(SIGINT, WebServer::closeAllThenExit);
    signal(SIGPIPE, SIG_IGN);
    setRealToVirt();
	std::vector<size_t> indices = extractVirtualHostsIndices();
    std::cout << "[INFO] Total number of servers: " << config.size()
        << " . Total number of virtual hosts " << indices.size()
        << ".\n";
    for (size_t i = 0; i < config.size(); ++i)
    {
        auto it = std::find(indices.begin(), indices.end(), i);
        if (it != indices.end())
            continue;
        _servers.emplace_back(std::make_unique<Server>(config.getAll(), i));
        std::cout << "[INFO] Server " << i << " is created.\n";
        _servers.back()->setVirthostList(_real_to_virt[i]);
        _servers.back()->setVirthostMap();
    }
    for (auto &srv : _servers)
    {
        srv->start(_fds);
        _fds_ptr = &_fds;
        std::cout << "[INFO] Server " << srv->index << " is started with port " << srv->get_port() << "\n";
    }
}

bool WebServer::fd_is_server(int fd)
{
	for (auto &srv : _servers)
		if (fd == srv->get_fd())
		{
			srv->accept_new_connection(_fds);
			return (true);
		}
	return (false);
}

void WebServer::setCgiToPoll(pollfd &pfd, Client *client)
{
	std::cout << "[INFO] CGI is done reading" << std::endl;
	_fds.push_back({client->get_cgi_fd(), POLLIN, 0});
	_cgi_clients.emplace(client->get_cgi_fd(), &pfd);
}

int WebServer::fd_is_client(pollfd &pfd)
{
	Client *client = nullptr;
    
	for (auto &srv : _servers)
		if ((client = srv->get_client(pfd.fd)))
		{
            if (pfd.revents & POLLOUT)
            {
                // std::cout << " pollout" << std::endl;
                if (!client->send_response())
                    return 0;
                client->close_connection();
                return 1;
            }
            if (pfd.revents & POLLIN)
            {
                // std::cout << " pollin" << std::endl;
			    int ret = client->handle_request();
                if (ret == 0)
                    pfd.events |= POLLOUT;
				if (ret == 2)
					setCgiToPoll(pfd, client);
                if (ret == -1)
                    client->close_connection();
                return ret == 1 ? 0 : ret == -1 ? 1 : ret;
            }
		}
	return -1;
}

bool WebServer::fd_is_cgi(int fd)
{
	auto it = _cgi_clients.find(fd);
	if (it == _cgi_clients.end())
		return false;
	it->second->events |= POLLOUT;
	std::cout << "[INFO] CGI is ready to write" << std::endl;
	_cgi_clients.erase(fd);
	return true;
}

std::vector<pollfd>::iterator WebServer::shutdown(std::vector<pollfd>::iterator &it)
{
	for (auto &srv : _servers)
		if (it->fd == srv->get_fd())
			return (++it);
	if (_cgi_clients.find(it->fd) != _cgi_clients.end())
	{
		_cgi_clients.erase(it->fd);
		return (_fds.erase(it));
	}
	for (auto &srv : _servers)
		if (Client *client = srv->get_client(it->fd))
		{
				client->close_connection();
				return (_fds.erase(it));
		}
	return (_fds.erase(it));
}

void WebServer::removeAllSocketsAndCGI()
{
	std::cout << "[INFO] Poll timeout" << std::endl;
	for (std::vector<pollfd>::iterator it = _fds.begin(); it != _fds.end();)
	{
		if (it->revents == 0)
			it = shutdown(it);
		else
			it++;
	}
}
void WebServer::run()
{
	int status;
	_fds.reserve(100);
	while (WebServer::running)
	{
		std::cout << "Waiting for action... - size of pollfd vector: " << _fds.size() << std::endl;
		int poll_result = poll(_fds.data(), _fds.size(), 15000);
		std::cout << "poll result: " << poll_result << std::endl;
		for (pollfd &pfd : _fds)
			std::cout << "fd: " << pfd.fd << " events: " << pfd.events << " revents: " << pfd.revents << " Address of object: " << &pfd << std::endl;
		if (poll_result == -1)
			return (perror("poll"));
		if (poll_result == 0)
		{
			removeAllSocketsAndCGI();
			continue;
		}
		for (std::vector<pollfd>::iterator it = _fds.begin(); it != _fds.end();)
		{
			if (it->revents & POLLHUP)
			{
				it = shutdown(it);
				continue;
			}
			if (it->revents & (POLLIN|POLLOUT))
			{
				if (fd_is_server(it->fd))
					break;
				if (fd_is_cgi(it->fd))
				{
					std::cout << "[INFO] erasing cgi from pollfd fd: " << it->fd << std::endl;
					it = _fds.erase(it);
					continue;
				}
				try
				{
					if ((status = fd_is_client(*it)) >=0)
					{
						if (status == 2)
							break;
						if (status == 1)
						{
							std::cout << "[INFO] erasing client from pollfd" << std::endl;
							it = _fds.erase(it);
							continue;
						}
					}
				}
				catch (std::exception *e)
				{
					it = shutdown(it);
					std::cerr << "[ERROR] Exception caught: " << e->what() << " - is deleted from poll" << std::endl;
					continue;
				}
			}
            it++;
		}
    }
}
