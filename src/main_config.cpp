
#include "WebServer.hpp"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: ./webserv [configuration file]" << std::endl;
        return 1;
    }

	WebServer webserv;
	webserv.setup(argv[1]);
	webserv.run();
    return 0;
}