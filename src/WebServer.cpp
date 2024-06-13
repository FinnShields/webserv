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
			bool connection_accepted = false;
			if (pfdit->revents & POLLIN)
			{
				for (Server &srv : _servers)
					if (pfdit->fd == srv._server_fd)
					{
						srv.accept_new_connection(_fds);
						connection_accepted = true;
						break ;
					}
				if (connection_accepted)
					break;
				for (Server &srv : _servers)
					for (Client &client : srv._clients)
						if (client.get_socket_fd() == pfdit->fd)
						{
							client.handle_request(srv);
							client.close_connection(srv, _fds, pfdit);
							break ;
						}
				
				break;
			}
			pfdit++;
		}
    }
}
