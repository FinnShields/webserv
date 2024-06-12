#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h> // For close
#include <stdexcept>
#include <vector>
#include <poll.h>
#include <algorithm>

#define DEFAULT_PORT 8080
#define MAX_BUFFER_SIZE 2048

class WebServer
{
	private:
		std::vector<pollfd> _fds;
		int _server_fd;
		struct sockaddr_in _address;
		int _opt = 1;
		int _addrlen = sizeof(_address);
		int _port;

		void parse_file(std::string filename);
		void setup_socket();
		void start_listen();
		void accept_new_connection();
		void handle_client(int socket);
		WebServer(WebServer &copy);
		WebServer &operator=(WebServer &assignment);

	public:
		void setup(std::string filename);
		void run();
		WebServer();
		~WebServer();
};

#endif