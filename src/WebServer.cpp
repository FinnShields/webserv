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
                srv._port = std::stoi(line.substr(line.find("=") + 1));
        }
		_servers.push_back(srv);
        config_file.close();
		return ; 
    }
	std::cout << "No cfg file, using default" << std::endl;
	Server srv;
	srv._port = DEFAULT_PORT;
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
		if (fd == srv._server_fd)
		{
			srv.accept_new_connection(_fds);
			return (true);
		}
	return (false);
}

void WebServer::fd_is_client(int fd)
{
	for (Server &srv : _servers)
	{
		for (Client &client : srv._clients)
			if (client.get_socket_fd() == fd)
			{
				client.handle_request(srv);
				client.close_connection(srv);
				return ;
			}
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
