#include "Client.hpp"


Client::Client(int fd) : _fd(fd) {}
Client::Client(const Client &copy) : _fd(copy._fd) {}
Client::~Client() {};

int Client::get_socket_fd()
{
    return (_fd);
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

void Client::handle_request(Server srv)
{
	(void) srv;
    
    char _buffer[MAX_BUFFER_SIZE] = {0};
	Request request;
	if (recv(_fd, &_buffer, MAX_BUFFER_SIZE, 0) < 0)
		perror("Recv error");
	request.parse(_buffer);
	std::cout << _buffer << std::endl;
	std::string method = request.get("method");
	std::string dir = request.get("request-target");
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
	if (send(_fd, response.c_str(), response.size(), 0) < 0)
		perror("Send error");
}

void Client::close_connection(Server &srv)
{
    close(_fd);
    srv.remove_client(_fd);
}