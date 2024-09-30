/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   WebServer.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:22:14 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/30 15:12:39 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"
#include "Config.hpp"
#include "WebServer.hpp"

int WebServer::running = 1;

WebServer::~WebServer() {}

WebServer::WebServer(std::vector<t_server>& data):config(Config(data, 0)) {}

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
	_fds.reserve(1000);
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
        std::cout << "[INFO] Server " << srv.index << " is started with port " << srv.get_port() << "\n";
    }
}

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

void WebServer::run()
{
	while (running)
	{
		std::cout << "Waiting for action - polls: " << _fds.size() << "\r" << std::flush;
		int poll_result = poll(_fds.data(), _fds.size(), POLLTIMEOUT);
		// for (pollfd &pfd : _fds)
		// 	std::cout << "fd: " << pfd.fd << " events: " << pfd.events << " revents: " << pfd.revents << " Address of object: " << &pfd << std::endl;
		if (poll_result == -1)
			return (perror("poll"));
		checkTimer(SOCKETTIMEOUT);
		if (poll_result == 0)
			continue;
		try
		{
			if (iterateAndCheckIncomingConnections() == 0)
				iterateAndRunActiveFD();
			usleep(1000);
		}
		catch (std::vector<pollfd>::iterator &it)
		{
			for (Server &srv : _servers)
				if (Client *client = srv.get_client(it->fd))
					client->close_connection();
			it = _fds.erase(it);
		}
    }
	cleanexit();
}

int WebServer::iterateAndCheckIncomingConnections()
{
	for (std::vector<pollfd>::iterator it = _fds.begin(); it != _fds.end(); it++)
	{
		if (it->revents)
			return fd_is_server(it->fd);
	}
	return 0;
}

void WebServer::iterateAndRunActiveFD()
{
	static std::vector<pollfd>::iterator it = _fds.begin();
	int status = -1;
	auto lastElement = it == _fds.begin() ? _fds.end() : it - 1;
	while (it != lastElement)
	{
		if (it == _fds.end())
			it = _fds.begin();
		try
		{
			if (it->revents & (POLLIN|POLLOUT|POLLHUP|POLLNVAL))
			{
				if ((status = fd_is_cgi(*it)) < 0)
					if ((status = fd_is_cgiwrite(*it)) < 0)
						status = fd_is_client(*it);
				it = (status == 1) ? _fds.erase(it) : it + 1;
				return;
			}
			it++;
		}
		catch (const std::exception& e)
		{
			std::cerr << "[ERROR] Exception caught: " << e.what() << std::endl;
			throw it;
		}
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
			std::cout << "[INFO] Client fd: " << pfd.fd << " : " << pfd.revents << " ";
            if (pfd.revents & POLLOUT)
				if (fd_is_client_write(pfd, client) == 1)
					return 1;
            if (pfd.revents & POLLIN)
				if (fd_is_client_read(pfd, client) == 1)
					return 1;
			if (pfd.revents & (POLLNVAL | POLLHUP))
			{
				client->close_connection();
				return 1;
			}
			return 0;
		}
	return -1;
}

int WebServer::fd_is_client_read(pollfd &pfd, Client *client)
{
	int ret = client->handle_request();
	if (ret == 2)
	{
		if (client->get_cgi_fd() == -1)
		{
			std::cout << "[ERROR] CGI Pipe fd is invalid" << std::endl;
			ret = 0;
		}
		else if (_cgi_readfd_clients.find(client->get_cgi_fd()) == _cgi_readfd_clients.end())
		{
			addCGItoPollfd(pfd, client);
			std::cout << "Added to poll" << std::endl;
			return 2;
		}
	}
	if (ret == 0)
		pfd.events |= POLLOUT;
	if (ret == -1)
	{
		if (client->shouldCloseConnection())
		{
			client->close_connection();
			return 1;
		}
		client->resetForNextRequest();
		pfd.events = POLLIN;
	}
	return 0;
}

void WebServer::addCGItoPollfd(pollfd &pfd, Client *client)
{
	_fds.push_back({client->get_cgi_fd(), POLLIN, 0});
	client->setcgiireadpfd(&_fds.back());
	_cgi_readfd_clients.emplace(client->get_cgi_fd(), &pfd);
	_fds.push_back({client->getCGIwritefd(), 0, 0});
	_cgi_writefd_clients.emplace(client->getCGIwritefd(), &pfd);
}


int WebServer::fd_is_client_write(pollfd &pfd, Client* client)
{
	int ret = client->send_response();
	if (ret == 2) 
		pfd.events &= ~POLLOUT;
	if (ret == 1)
	{
		if (client->shouldCloseConnection())
		{
			client->close_connection();
			return 1;
		}
		client->resetForNextRequest();
		pfd.events = POLLIN;
	}
	return 0;
}

int WebServer::fd_is_cgi(pollfd pfd)
{
	auto it = _cgi_readfd_clients.find(pfd.fd);
	if (it == _cgi_readfd_clients.end())
		return -1;
	std::cout << "[INFO] CGI read fd: " << pfd.fd << " : " << pfd.revents << " :";
	Client *client = nullptr;
    
	for (Server &srv : _servers)
		if ((client = srv.get_client(it->second->fd)))
		{
			if ((pfd.revents & (POLLIN | POLLHUP) && client->readFromCGI()) )
				it->second->events |= POLLOUT;
			else if (pfd.revents & (POLLNVAL | POLLHUP))
			{
				it->second->events |= POLLOUT;
				close(pfd.fd);
				_cgi_readfd_clients.erase(pfd.fd);
				return 1;
			}
			return 0;
		}
	if (!client)
	{
		close(pfd.fd);
		_cgi_readfd_clients.erase(pfd.fd);
		return 1;
	}
	return 0;
}

int WebServer::fd_is_cgiwrite(pollfd &pfd)
{
	auto it = _cgi_writefd_clients.find(pfd.fd);
	if (it == _cgi_writefd_clients.end())
		return -1;
	std::cout << "[INFO] CGI write fd: " << pfd.fd << " : " << pfd.revents << " : ";
	Client *client = nullptr;

	for (Server &srv : _servers)
		if ((client = srv.get_client(it->second->fd)))
		{
			if (pfd.revents & POLLOUT)
				if (client->writeToCgi() == 0)
					pfd.events = 0;
			if (client->isRequestComplete() || pfd.revents & POLLHUP || pfd.revents & POLLNVAL)
			{
				_cgi_writefd_clients.erase(pfd.fd);
				close(pfd.fd);
				return 1;
			}
			return 0;
		}
	std::cout << "[ERROR] client is not found, deleting the fd" << std::endl;
	_cgi_writefd_clients.erase(pfd.fd);
	close(pfd.fd);
	return 1;
}

bool WebServer::checkTimer(int timeout_seconds)
{
	Client *client;
	bool timedout = false;
	static time_t lastCheck = std::time(NULL);
	
	std::cout << "[INFO] TIMER lastCheck: " << std::difftime(std::time(NULL), lastCheck) << "s ago" << std::endl;
	if (std::difftime(std::time(NULL), lastCheck) < 30)
	{
		// std::cout << "[INFO] TIMER less than 30 sec since last, wont check" << std::endl;
		return false;
	}
	for (std::vector<pollfd>::iterator it = _fds.begin(); it != _fds.end(); it++)
		if (it->revents == 0)
			for (Server &srv : _servers)
				if ((client = srv.get_client(it->fd)))
				{
					if (client->timeout(timeout_seconds))
					{
						timedout = true;
						it->events = POLLOUT;
					}
					break;
				}
	lastCheck = std::time(NULL);
	return timedout;
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
