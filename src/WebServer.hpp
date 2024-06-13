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

#include "Server.hpp"

#define DEFAULT_PORT 8080
#define MAX_BUFFER_SIZE 2048

class WebServer
{
	private:
		std::vector<pollfd> _fds;
		std::vector<Server> _servers;

		void parse_file(std::string filename);
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