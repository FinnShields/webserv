#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include "Request.hpp"

class Request;

class Cgi
{
    private:
        Request& _request;
        std::map<std::string,std::string> _env_map;

        Cgi();
    public:
        Cgi(Request&);
        Cgi(const Cgi&);
        Cgi &operator=(const Cgi&);
        ~Cgi();
        
        void doCgi(std::string);
        void setEnvMap();
};

#endif
