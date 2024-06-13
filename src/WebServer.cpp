#include "WebServer.hpp"
#include "Request.hpp"

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
					}
				if (connection_accepted)
					break;
				handle_client(pfdit->fd);
				close(pfdit->fd);
				pfdit = _fds.erase(pfdit);
				break;
			}
			pfdit++;
		}
    }
}

static std::string load_index()
{
	std::ifstream file("www/index.html");
	if (!file.is_open())
		return ("HTTP/1.1 404 Not Found\nContent-Type: text/plain\n\nError: index.html not found");
	
	std::stringstream buffer;
	buffer << file.rdbuf();
	file.close();

	return ("HTTP/1.1 200 OK\nContent-Type: text/html\n\n" + buffer.str());
}

void WebServer::handle_client(int socket)
{
	char _buffer[MAX_BUFFER_SIZE] = {0};
	Request request;
	if (recv(socket, &_buffer, MAX_BUFFER_SIZE, 0) < 0)
		perror("Recv error");
	request.parse(_buffer);
	std::string method = request.getMethod();
	std::string dir = request.getDir();
	if (!method.empty())
	{
		std::cout << "Received a request:" << std::endl;
		request.display();
	}
	else
		std::cout << "Connection cancelled (empty method)" << std::endl;
	std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nYour request: " + method + " " + dir;
	if (method == "GET")
		if (dir == "/")
			response = load_index();
	if (send(socket, response.c_str(), response.size(), 0) < 0)
		perror("Send error");
}
