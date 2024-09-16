/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:14 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/13 12:40:57 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "Config.hpp"
#include "WebServer.hpp"

static std::vector<pollfd> *_fds_ptr;
int WebServer::running = 1;

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

// void WebServer::closeAllThenExit(int signal)
// {
//     std::vector<pollfd> _fds;
//     _fds = *_fds_ptr;
//     if (signal == SIGINT)
//         std::cout << " -- Closing due to SIGINT (ctl-c) " << std::endl;
//     for (size_t i = 0; i < _fds.size(); i ++)
//         close(_fds[i].fd);
//     exit(130);
// }

void WebServer::setExit(int signal)
{
	if (signal == SIGINT)
		std::cout << " -- Closing due to SIGINT (ctl-c) " << std::endl;
	WebServer::running = 0;	
}

void WebServer::setup()
{
    signal(SIGINT, WebServer::setExit);
    signal(SIGPIPE, SIG_IGN);
    setRealToVirt();
	_fds.reserve(100);
	std::vector<size_t> indices = extractVirtualHostsIndices();
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
        _fds_ptr = &_fds;
        std::cout << "[INFO] Server " << srv.index << " is started with port " << srv.get_port() << "\n";
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

int WebServer::fd_is_client(pollfd &pfd)
{
	Client *client = nullptr;
    
	for (Server &srv : _servers)
		if ((client = srv.get_client(pfd.fd)))
		{
            if (pfd.revents & POLLOUT)
            {
                if (!client->send_response())
                    return 0;
                client->close_connection();
                return 1;
            }
            if (pfd.revents & POLLIN)
            {
			    int ret = client->handle_request();
                if (ret == 0)
                    pfd.events |= POLLOUT;
				if (ret == 2)
				{
					std::cout << "[INFO] CGI is done reading" << std::endl;
					_fds.push_back({client->get_cgi_fd(), POLLIN, 0});
					_cgi_clients.emplace(client->get_cgi_fd(), &pfd);
					return 2;
				}
                if (ret == -1)
                {
                    client->close_connection();
                    return 1;
                }
                return 0;
            }
			client->close_connection();
			return 1;
		}
	return -1;
}

int WebServer::fd_is_cgi(pollfd pfd)
{
	auto it = _cgi_clients.find(pfd.fd);
	if (it == _cgi_clients.end())
		return -1;
	Client *client = nullptr;
    
	for (Server &srv : _servers)
		if ((client = srv.get_client(it->second->fd)))
		{
			std::cout << "[INFO] Reads from CGI" << std::endl;
			if (pfd.revents & (POLLNVAL) || client->readFromCGI())
			{
				std::cout << "[INFO] CGI has finished" << std::endl;
				it->second->events |= POLLOUT;
				_cgi_clients.erase(pfd.fd);
				return 1;
			}
			return 0;
		}
	if (client == NULL)
	{
		_cgi_clients.erase(pfd.fd);
		return 1;
	}
	return 0;
}
bool WebServer::checkTimer(int timeout_seconds)
{
	Client *client;
	bool timedout = false;
	
	std::cout << "[INFO] Check timers" << std::endl;
	for (std::vector<pollfd>::iterator it = _fds.begin(); it != _fds.end(); it++)
		if (it->revents == 0)
			for (Server &srv : _servers)
				if ((client = srv.get_client(it->fd)))
				{
					if (client->timeout(timeout_seconds))
					{
						timedout = true;
						it->events = POLLOUT;
						std::cout << "[INFO] Client set to POLLOUT" << std::endl;
					}
					break;
				}
	return timedout;
}


bool WebServer::eraseAndContinue(std::vector<pollfd>::iterator &it, std::string what)
{
	std::cout << "[INFO] erasing " << what << " from pollfd fd: " << it->fd << std::endl;
	it = _fds.erase(it);
	return true;
}

void WebServer::iterateAndRunActiveFD()
{
	int status;
	for (std::vector<pollfd>::iterator it = _fds.begin(); it != _fds.end();)
	{
		try
		{
			if (it->revents & (POLLIN|POLLOUT|POLLHUP|POLLNVAL))
			{
				if (fd_is_server(it->fd))
					return;
				if ((status = fd_is_client(*it)) >=0)
				{
					if (status == 2)
						return;
					if (status == 1 && eraseAndContinue(it, "client"))
						continue;
				}
				else if ((status = fd_is_cgi(*it)) >= 0)
					if (status == 1 && eraseAndContinue(it, "cgi"))
						continue;
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "[ERROR] Exception caught: " << e.what() << std::endl;
			throw it;
		}
		it++;
	}
}

void WebServer::run()
{
	while (running)
	{
		std::cout << "Waiting for action... - size of pollfd vector: " << _fds.size() << std::endl;
		int poll_result = poll(_fds.data(), _fds.size(), POLLTIMEOUT);
		for (pollfd &pfd : _fds)
			std::cout << "fd: " << pfd.fd << " events: " << pfd.events << " revents: " << pfd.revents << " Address of object: " << &pfd << std::endl;
		if (poll_result == -1)
			return (perror("poll"));
		if (!checkTimer(SOCKETTIMEOUT) && poll_result == 0)
			continue;
		std::cerr << "No timed out" << std::endl;
		try
		{
			iterateAndRunActiveFD();
		}
		catch (std::vector<pollfd>::iterator it)
		{
			for (Server &srv : _servers)
				if (Client *client = srv.get_client(it->fd))
					client->close_connection();
			_fds.erase(it);
		}
    }
	cleanexit();
}

void WebServer::cleanexit()
{
	for (std::vector<pollfd>::iterator it = _fds.begin(); it != _fds.end();)
	{
		for (Server &srv : _servers)
			if (srv.get_client(it->fd))
			{
				srv.remove_client(it->fd);
				break;
			}
		close(it->fd);
		it = _fds.erase(it);
	}
}
