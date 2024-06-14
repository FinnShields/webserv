
#include "ParceConf.hpp"

int main() {


	WebServer webserv;
	webserv.setup(argv[1]);
	webserv.run();
    return 0;
}
