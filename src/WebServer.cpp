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

#include "Parser.hpp"
#include "Config.hpp"
#include "WebServer.hpp"

WebServer::WebServer() {}

WebServer::~WebServer() {}

/*
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
		_servers.push_back(srv);
        config_file.close();
		return ; 
    }
	std::cout << "No cfg file, using default" << std::endl;
	Server srv;
	srv.set_port(DEFAULT_PORT);
	_servers.push_back(srv);
}
*/

void WebServer::parse_file(std::string filename)
{
	Parser data(filename);
	//Config conf(data.get());
	//conf.get();
	//vector<t_server> data = data.get
	std::cout
		<< "----- EXTRACTED DATA ------------\n"
		<< data.get()
		<< "----- END of DATA ---------------\n";
	// The following part to be refactored and moved to Parser and/or Server. 
	std::cout << "Number of servers: " << data.size() << "\n";
	for (size_t i = 0; i < data.size(); ++i) {
		Server srv(data.get(), i);
		std::cout << srv._config.get(0, "main", "newkey", 0) 
			<< "\n my index=" << srv._server_index <<  "\n";
		std::string port_str = data.get(i, "main", "listen", 0);
		if (port_str.empty())
			srv.set_port(DEFAULT_PORT);
		else
			srv.set_port(std::stoi(port_str));
		_servers.push_back(srv);
		std::cout << "Server " << i << " is initialized with port " << srv.get_port() << "\n";
	}
}


void WebServer::setup(std::string filename)
{
	try
	{
		int i = -1;
		parse_file(filename);
		for (Server &srv : _servers){
			std::cout << "Starting server " << ++i << " with port " << srv.get_port() << "\n"; 
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
