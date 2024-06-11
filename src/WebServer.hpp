#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h> // For close
#include <stdexcept>

#define DEFAULT_PORT 8080
#define MAX_BUFFER_SIZE 2048

class WebServer
{
	private:
		int _server_fd, _new_socket;
		struct sockaddr_in _address;
		int _opt = 1;
		int _addrlen = sizeof(_address);
		int _port;

		void parse_file(std::string filename);
		void setup_socket();
		void start_listen();
		WebServer(WebServer &copy);
		WebServer &operator=(WebServer &assignment);

	public:
		void setup(std::string filename);
		void run();
		WebServer();
		~WebServer();
};

#endif