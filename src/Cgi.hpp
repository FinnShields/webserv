#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <cstring>
//#include <unistd.h> 
//#include <fcntl.h>  
#include <sys/wait.h>


#include "Server.hpp"
#include "Request.hpp"

//#define STDOUT_FILENO  1
//#define STDIN_FILENO  0
class Server;
class Request;

class Cgi
{
    private:
        Request& _request;
        const Server& _server;
        std::string _body;
        std::map<std::string,std::string> _env_map;
        char** _argv;
        char** _envp;
        std::string _response;
        int _status;

        Cgi();
        void setEnvMap();
        void setEnv();
        void cleanEnv();
        std::string readFromFd(int);
    public:
        Cgi(Request&, const Server&);
        Cgi(const Cgi&);
        Cgi &operator=(const Cgi&);
        ~Cgi();
        
        void start();
        void runCmd();
        std::string getResponse();
};

#endif
