#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <cstring>
//#include <unistd.h> 
//#include <fcntl.h>  
#include <ctime>
#include <algorithm>
#include <sys/wait.h>


#include "Server.hpp"
#include "Request.hpp"

//#define STDOUT_FILENO  1
//#define STDIN_FILENO  0
#define CGI_TIMEOUT	 5
class Server;
class Request;

class Cgi
{
    private:
        Request& _request;
        const Server& _server;
        std::string& _body;
        std::string _target;
        std::string _ext;
        size_t _pos_query;
        size_t _pos_cgi;
        size_t _pos_info;
        
        std::map<std::string,std::string> _env_map;
        char** _argv;
        char** _envp;
        std::string _response;
        int _fd_from_cgi[2];
        int _fd_to_cgi[2];
        int _pid;
        int _status;

        Cgi();
        bool isImplemented();
        void setEnvMap();
        void setEnv();
        void cleanEnv();
        std::string readFromFd(int);
        int _access();
        void _runChildCgi();
        bool _wait();
    public:
        Cgi(Request&, const Server&);
        Cgi(const Cgi&);
        Cgi &operator=(const Cgi&);
        ~Cgi();
        
        void start();
        void runCmd();
        std::string getResponse();
        int getStatus();
        
};

#endif
