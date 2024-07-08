#include "Cgi.hpp"

//Cgi::Cgi();

Cgi::Cgi(const Cgi& other):
    _request(other._request),
    _server(other._server),
    _body(other._body){
    _argv = new char*[4];
    };

Cgi::Cgi(Request& r, const Server& s):_request(r),_server(s),
    _body(_request.getRef("body")){
        _argv = new char*[4];
    };

Cgi& Cgi::operator=(const Cgi&){
    return *this;
}

Cgi::~Cgi(){
    cleanEnv();
    int i = 0;
    if (_argv){
        while (_argv[i]){
            free(_argv[i]);
            i++;
        }
        free(_argv);
    }
};

void Cgi::start(){
    std::cout << "This is Cgi request. Work in progress.\n";
    std::cout << "Method =>" << _request.get("method") << "<=\n";
    std::cout << "Target =>" << _request.get("target") << "<=\n";
    setEnvMap();
    setEnv();
    runCmd();
    std::cout << "status =" << _status << "\n";
        /*
    try{
        runCmd();
    }
    catch (const std::runtime_error& e){
        std::cerr << e.what() << "\n";
        _response = RESPONSE_500;
    }
    catch (...) {
        std::cerr << "Caught unknown exception." << std::endl;
        _response = RESPONSE_500;
    }
    std::cout << "status =" << _status << "\n";
    */
}

/*   // types of error in Cgi
 502 "Bad Gateway: The server received an empty response from the CGI script.";
"PATH NOT FOUND", 404
"PERMISSION DENIED", 403
"UNKNOWN METHOD", 405
"INTERNAL SERVER ERROR", 500
"NOT IMPLEMENTED", 501
OK:
"TEAPOT", 418

If found and executable, the server runs the script.
If not found, a 404 error is returned.
If found but not executable, a 403 error or similar may be returned.
If execution fails, a 500 error may be returned.
*/

std::string Cgi::getResponse() {
    return _response;
}

std::string Cgi::readFromFd(int fd) {
    std::string result;
    char buffer[1024];
    ssize_t size;
    while ((size = read(fd, buffer, sizeof(buffer))) > 0) {
        result.append(buffer, size);
    }
    if (size < 0) {
        throw std::runtime_error("readFromFd: read error occured!");
    }
    return result;
}

void Cgi::runCmd(){
    _argv[0] = strdup(_env_map["SCRIPT_FILENAME"].c_str());
    int fd_from_cgi[2];
    int fd_to_cgi[2];
    if (pipe(fd_to_cgi) == -1)
        throw std::runtime_error("pipe error occurred!");
    if (pipe(fd_from_cgi) == -1){
        close(fd_to_cgi[0]);
        close(fd_to_cgi[1]);
        throw std::runtime_error("pipe error occurred!");
    }
    pid_t pid = fork();
    if (pid){
        close(fd_to_cgi[0]);
        close(fd_from_cgi[1]);
        write(fd_to_cgi[1], _body.c_str(), _body.size());
        close(fd_to_cgi[1]);
        _response = readFromFd(fd_from_cgi[0]);
        close(fd_from_cgi[0]);
    }
    else {
        std::cout << "CGI: execve for " << _argv[0] << "\n";
        close(fd_to_cgi[1]);
        close(fd_from_cgi[0]);
        dup2(fd_to_cgi[0], 0);
        dup2(fd_from_cgi[1], 1);
        execve(_argv[0], _argv, _envp);
        close(fd_to_cgi[0]);
        close(fd_from_cgi[1]);
        throw std::runtime_error("CGI:execve error occurred!");
    }
    int status;
    waitpid(pid, &status, 0);
    _status = (status & 0xff00) >> 8;
    //if (WIFEXITED(status))
    //    _status = WEXITSTATUS(status);
    if (_response.empty())
			_response = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/plain\n";
}

void Cgi::cleanEnv(){
    if (!_envp)
        return ;
    int i = 0;
    while (_envp[i]){
        delete[] _envp[i];
        _envp[i] = nullptr;
    }
    delete[] _envp;
    _envp = nullptr;
}

void Cgi::setEnv(){
    _envp = new char*[_env_map.size() + 1];
    char* line_pnt;
    std::string line;
    int i = 0;
    for (auto& [key, value] : _env_map){
        line = key + "=" + value;
        line_pnt = new char[line.size() + 1];
        std::strcpy(line_pnt, line.c_str());
        _envp[i] = line_pnt;
        ++i;
    }
}

void Cgi::setEnvMap(){
    _env_map["AUTH_TYPE"] = "basic";
    _env_map["CONTENT_LENGTH"] = _request.get("Content-Length");
	_env_map["CONTENT_TYPE"] = _request.get("Content-Type");
    _env_map["DOCUMENT_ROOT"] = _server.config.getFirst("main","root","");
    _env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env_map["HTTP_COOKIE"] = _request.get("Cookie");
    _env_map["HTTP_USER_AGENT"] = _request.get("User-Agent");
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
    // SCRIPT_NAME  ???
    _env_map["PATH_TRANSLATED"] = _env_map["DOCUMENT_ROOT"] + _env_map["PATH_INFO"];
    _env_map["REDIRECT_STATUS"] = "200";
    _env_map["REQUEST_METHOD"] = _request.get("method");
    _env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env_map["SERVER_SOFTWARE"] = "Webserv_FAB/1.0";
    _env_map["SERVER_NAME"] = _request.get("Host");
    size_t pos = _env_map["SERVER_NAME"].find(':');
    if (pos != std::string::npos)
        _env_map["SERVER_PORT"] = _env_map["SERVER_NAME"].substr(pos + 1);
    else
        _env_map["SERVER_PORT"] = "80";
    for (auto it = _env_map.begin(); it != _env_map.end();) {
        if (it->second.empty()) {
            it = _env_map.erase(it);
        } else
            ++it;
    }
    

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