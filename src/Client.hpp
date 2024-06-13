#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <fstream>
#include <sstream> 
#include <unistd.h>
#include "Server.hpp"
#include "Request.hpp"

#define MAX_BUFFER_SIZE 2048

class Server;

class Client
{
    private:
        int _fd;

    public:
        Client(int socket_fd);
        Client(const Client &copy);
        ~Client();
        
        void handle_request(Server srv);
        void close_connection(Server &srv, std::vector<pollfd> &fds, std::vector<pollfd>::iterator &it);
        
        int get_socket_fd();
};

#endif