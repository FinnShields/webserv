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
	}
	catch(char const *e)
	{
		perror(e);
		exit(EXIT_FAILURE);
	}
	
}
static void parse(char *_buffer)
{
		// Assuming _buffer contains the received data
	std::istringstream request(_buffer);
	std::string line;

	// The first line of the request should be the request line
	std::getline(request, line);
	std::cout << "Request line: " << line << std::endl;

	// Then we have the headers
	std::cout << "\nHeaders:" << std::endl;
	while (std::getline(request, line) && line != "\r")
		std::cout << line << std::endl;

	std::cout << "\nBody: " << std::endl;
	while (std::getline(request, line))
		std::cout << line << std::endl;
}


void WebServer::run()
{
	while (1)
	{
		std::cout << "\nWaiting for a connection..." << std::endl;
		try
		{
			if ((_new_socket = accept(_server_fd, (struct sockaddr *)&_address, (socklen_t*)&_addrlen)) < 0)
				throw ("accept");
			char _buffer[MAX_BUFFER_SIZE] = {0};
			if (recv(_new_socket, &_buffer, MAX_BUFFER_SIZE, 0) < 0)
				throw ("Recv error");
			parse(_buffer);
			std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nHello, World!";
			if (send(_new_socket, response.c_str(), response.size(), 0) < 0)
				throw ("Send error");
		}
		catch(const char *e) 
		{
			perror(e);
		}
		close(_new_socket);
    }
}
