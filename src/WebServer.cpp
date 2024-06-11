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
			{
                _port = std::stoi(line.substr(line.find("=") + 1));
				std::cout << "Port: " << _port << std::endl;
            }
        }
        config_file.close();
		return ; 
    }
	std::cout << "No cfg file, using default" << std::endl;
	std::cout << "Port: 8080" << std::endl;
}

void WebServer::setup_socket()
{
	//Creates the socket
	if ((_server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	//Attaches the socket (optional?)
	if (setsockopt(_server_fd, SOL_SOCKET, SO_REUSEADDR, &_opt, sizeof(_opt))) 
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
    }
	//Binds the socket
	_address.sin_family = AF_INET;
    _address.sin_addr.s_addr = INADDR_ANY;
    _address.sin_port = htons(_port);
    if (bind(_server_fd, (struct sockaddr *)&_address, sizeof(_address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

void WebServer::start_listen()
{
	if (listen(_server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
    }
}

void WebServer::setup(std::string filename)
{
	parse_file(filename);
	setup_socket();
	start_listen();
}

void WebServer::run()
{
	while (1)
	{
		std::cout << "\nWaiting for a connection..." << std::endl;
		if ((_new_socket = accept(_server_fd, (struct sockaddr *)&_address, (socklen_t*)&_addrlen)) < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		if (recv(_new_socket, &_buffer, MAX_BUFFER_SIZE, 0) < 0)
			std::cout << "Recv error" << std::endl;
		else
			std::cout << "Recvicved info:\n" << _buffer << std::endl;
		
		std::string response = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nHello, World!";
		if (send(_new_socket, response.c_str(), response.size(), 0) < 0)
			std::cout << "Send error" << std::endl;
		
		close(_new_socket);
    }
}
