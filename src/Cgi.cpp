#include "Cgi.hpp"

//Cgi::Cgi();

Cgi::Cgi(const Cgi& other):_request(other._request){};

Cgi::Cgi(Request& r):_request(r){
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
    std::cout << "------ CGI ENV ---------.\n";
    for (auto& [key, value] : _env_map){
        std::cout << key << "=" << value << "\n";
    }
    std::cout << "------ END ---------.\n";
}

void Cgi::setEnvMap(){
    _env_map["AUTH_TYPE"] = "basic";
    _env_map["REDIRECT_STATUS"] = "200";
    _env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env_map["SERVER_SOFTWARE"] = "Webserv_FAB/1.0";
    _env_map["DOCUMENT_ROOT"] = "";  // no need to implement?!
    //_env_map["REMOTE_PORT"] = "";    // no need to implement?!
    
    _env_map["REQUEST_METHOD"] = _request.get("method");
	_env_map["CONTENT_LENGTH"] = _request.get("Content-Length");
	_env_map["CONTENT_TYPE"] = _request.get("Content-Type");
    _env_map["HTTP_COOKIE"] = _request.get("Cookie");
    _env_map["HTTP_USER_AGENT"] = _request.get("User-Agent");
	_env_map["SERVER_NAME"] = _request.get("Host");
    size_t pos = _env_map["SERVER_NAME"].find(':');
    if (pos != std::string::npos)
        _env_map["SERVER_PORT"] = _env_map["SERVER_NAME"].substr(pos);
    else
        _env_map["SERVER_PORT"] = "80";

    std::string target = _request.get("target");
    size_t pos_query = target.find('?');
    size_t pos_info = target.rfind('/', pos_query);
    size_t pos_cgi = target.find('/', 1);
    if (pos_query != std::string::npos)
        _env_map["QUERY_STRING"] = target.substr(pos_query + 1);
    else
        _env_map["QUERY_STRING"] = "";
    if (pos_info != pos_cgi){
        _env_map["PATH_INFO"] = target.substr(pos_info, pos_query - pos_info);
        _env_map["SCRIPT_FILENAME"] = _env_map["DOCUMENT_ROOT"] + target.substr(0, pos_info);
    }
    else{
        _env_map["PATH_INFO"] = "";
        _env_map["SCRIPT_FILENAME"] = _env_map["DOCUMENT_ROOT"] + target.substr(0, pos_query);
    }
    _env_map["PATH_TRANSLATED"] = _env_map["DOCUMENT_ROOT"] + _env_map["PATH_INFO"];
/*
    std::cout << pos_cgi << "\n";
    std::cout << pos_info << "\n";
    std::cout << pos_query << "\n";
    (void)pos_cgi;
    (void)pos_info;
    std::cout << _env_map["SCRIPT_FILENAME"] << "\n";
    std::cout << _env_map["PATH_INFO"] << "\n";
    std::cout << _env_map["QUERY_STRING"] << "\n";
    */    
}   

/*
   // /cgi-bin/script.cgi   /info   ?   query=python
    
    


DOCUMENT_ROOT: This is a server-specific configuration.
REMOTE_PORT:  This is information about the client's connection.
QUERY_STRING: The query string is part of the URL in the GET request, 
    not a header. It appears after the ? in the URL.
PATH_INFO:   This is additional path information in the URL.
PATH_TRANSLATED:  This is a server-specific mapping.
SCRIPT_FILENAME:   This is a server-specific configuration.
SERVER_NAME:  However, the Host header typically includes the server name 
    or IP address and the port number (if not the default port).
SERVER_PORT:  This is part of the URL in the request line and also implicitly included in the Host header if it's not the default port.


*/