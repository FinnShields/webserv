#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <iostream> //std::cout
#include <fstream> //Parse file
#include <sstream> 

#include <unistd.h> // For close
#include <vector>
#include <poll.h>
#include <algorithm>

#include "Server.hpp"

#define DEFAULT_PORT 8080

class WebServer
{
	private:
		std::vector<pollfd> _fds;
		std::vector<Server> _servers;

		void parse_file(std::string filename);
		WebServer(WebServer &copy);
		WebServer &operator=(WebServer &assignment);

	public:
		void setup(std::string filename);
		void run();
		WebServer();
		~WebServer();
};

#endif