#include "Cgi.hpp"

//Cgi::Cgi();

Cgi::Cgi(const Cgi& other):_request(other._request){};

Cgi::Cgi(const Request& r):_request(r){
    setEnvMap();
    doCgi("");
};

Cgi& Cgi::operator=(const Cgi&){
    return *this;
}

Cgi::~Cgi(){};

void Cgi::doCgi(std::string str){
    std::cout << "This is Cgi request. To be implemented.\n"
        << str << "\n";
}

void Cgi::setEnvMap(){
    _env_map["AUTH_TYPE"] = "basic";
    _env_map["REDIRECT_STATUS"] = "200";
    _env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env_map["SERVER_SOFTWARE"] = "Webserv_FAB/1.0";
    
  //  _env_map["REQUEST_METHOD"] = 
//	_env_map["CONTENT_LENGTH"] = 
//	_env_map["CONTENT_TYPE"] = 

/*
    _env_map["DOCUMENT_ROOT"] = 
    _env_map["HTTP_COOKIE"] = 
    _env_map["HTTP_USER_AGENT"] = 
	_env_map["REMOTE_PORT"] = 
    _env_map["QUERY_STRING"] = 
    _env_map["PATH_INFO"] =
	_env_map["PATH_TRANSLATED"] =
	_env_map["SCRIPT_FILENAME"] = 
	_env_map["SERVER_NAME"] = 
	_env_map["SERVER_PORT"] = 
*/	
}