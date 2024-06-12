#include "WebServer.hpp"

WebServer::WebServer()
{
	_port = DEFAULT_PORT;
}

WebServer::~WebServer()
{}

void WebServer::parse_file(std::string filename)
{
    std::ifstream config_file(filename);
    std::string line;
    if (config_file.is_open())
	{
		std::cout << "Reading file" << std::endl;
        while (getline(config_file, line))
		{
            if (line.find("port") != std::string::npos)
                _port = std::stoi(line.substr(line.find("=") + 1));
        }
        config_file.close();
		return ; 
    }
	std::cout << "No cfg file, using default" << std::endl;
}

void WebServer::setup_socket()
{
	//Creates the socket
	if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
		throw ("socket failed");
	//Attaches the socket (optional?)
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &_opt, sizeof(_opt))) 
		throw ("setsockopt");
	//Binds the socket
	_address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_port);
    if (bind(_server_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0)
        throw ("bind failed");
}

void WebServer::start_listen()
{
	if (listen(_server_fd, 3) < 0)
		throw ("listen");
	std::cout << "Start listenting to port: " << _port << std::endl;
}

void WebServer::setup(std::string filename)
{
	try
	{
		parse_file(filename);
		setup_socket();
		start_listen();

		pollfd server;
		server.fd = _server_fd;
		server.events = POLLIN;
		fds.push_back(server);
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
		std::cout << "\nWaiting for a connection... - size of pollfd vector: " << fds.size() << std::endl;
		int poll_result = poll(fds.data(), fds.size(), -1);
		if (poll_result == -1)
			return (perror("poll"));
		for (std::vector<pollfd>::iterator pfdit = fds.begin(); pfdit != fds.end();)
		{
			if (pfdit->revents & POLLIN)
			{
				if (pfdit->fd == _server_fd)
					accept_new_connection();
				else
				{
					handle_client(pfdit->fd);
					close(pfdit->fd);
					pfdit = fds.erase(pfdit);
					continue;
				}
			}
			pfdit++;
		}
    }
}

void WebServer::accept_new_connection()
{
	pollfd client;
	
	if ((client.fd = accept(_server_fd, (struct sockaddr *)&_address, (socklen_t*)&_addrlen)) < 0)
		return (perror("accept"));
	client.events = POLLIN;
	fds.push_back(client);
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
	if (recv(socket, &_buffer, MAX_BUFFER_SIZE, 0) < 0)
		return (perror("Recv error"));

	std::istringstream request(_buffer);
	std::string method, dir;
	std::getline(request, method, ' ');
	std::getline(request, dir, ' ');

	std::cout << "Recicved a " + method + " " + dir;
	if (method == "GET")
	{
		std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nYou sent a GET request for " + dir;
		if (dir == "/")
			response = load_index();
		if (send(socket, response.c_str(), response.size(), 0) < 0)
			perror("Send error");
	}
}