#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdexcept>
#include <map>
#include <vector>
#include <iostream>
#include <poll.h>

class Server 
{
    private:
        typedef std::map<std::string, std::string> config;

        std::map<std::string, config> _locations;
		struct sockaddr_in _address;
		int _opt = 1;
		int _addrlen = sizeof(_address);
        
        void setup_socket();
		void start_listen();
        
        Server &operator=(const Server &copy);
    
    public:
		int _server_fd;
		int _port;
        void start(std::vector<pollfd> &_fds);
        void accept_new_connection(std::vector<pollfd> &_fds);
        Server();
        Server (const Server &copy);
        ~Server();
};