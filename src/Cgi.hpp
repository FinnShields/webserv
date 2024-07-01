#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <cstring>
#include "Server.hpp"
#include "Request.hpp"

class Server;
class Request;

class Cgi
{
    private:
        Request& _request;
        const Server& _server;
        std::map<std::string,std::string> _env_map;
        char** _envp;

        Cgi();
        void setEnvMap();
        void setEnv();
        void cleanEnv();
    public:
        Cgi(Request&, const Server&);
        Cgi(const Cgi&);
        Cgi &operator=(const Cgi&);
        ~Cgi();
        
        void start();
        void runCmd();
};

#endif
