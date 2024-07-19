#include "Cgi.hpp"

//Cgi::Cgi();

Cgi::Cgi(const Cgi& other):
    _request(other._request),
    _server(other._server),
    _body(other._body),
    _target(other._target)
{
    _argv = new char*[4] {nullptr};
    _envp = nullptr;
}

Cgi::Cgi(Request& r, const Server& s):
    _request(r),
    _server(s),
    _body(_request.getRef("body")),
    _target(_request.get("target"))
{
    _argv = new char*[4] {nullptr};
    _envp = nullptr;
}

Cgi& Cgi::operator=(const Cgi&){
    return *this;
}

Cgi::~Cgi(){
    //std::cerr << "Cgi destructor call\n";
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

int Cgi::getStatus(){
    return _status;
}

void Cgi::start(){
    std::cout << "__________This is Cgi request._____________\n";
    std::cout << "Method =>" << _request.get("method") << "<=\n";
    std::cout << "Target =>" << _target << "<=\n";
    //std::cout << "Target =>" << _request.get("target") << "<=\n";
    if (!isImplemented())
    {
        std::cerr << "ext is not implemented\n";
        _response = "";
        _status = 501;
        return ;
    }
    setEnvMap();
    setEnv();
    //runCmd();
    //std::cout << "status =" << _status << "\n";*
    try{
        runCmd();
    }
    catch (const std::runtime_error& e){
        std::cerr << e.what() << "\n";
        _response = "";
        _status = 500;
    }
    catch (...) {
        std::cerr << "Caught unknown exception." << std::endl;
        _response = "";
        _status = 500;
    }
    std::cout << "_status =" << _status << "\n";
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

// types of error in Cgi
"NOT IMPLEMENTED", 501       -- checked in Response
"UNKNOWN METHOD", 405        -- checked in Response
"PATH NOT FOUND", 404        -- here
"PERMISSION DENIED", 403     -- here

 502 "Bad Gateway: The server received an empty response from the CGI script.";
"INTERNAL SERVER ERROR", 500
"TEAPOT", 418

If found and executable, the server runs the script.
If not found, a 404 error is returned.
If found but not executable, a 403 error or similar may be returned.
If execution fails, a 500 error may be returned.
*/

bool Cgi::isImplemented()
{
    size_t pos = _target.rfind('.');
    //std::cerr << "pos =" << pos << "\n";
    if (pos == std::string::npos)
        return false;
    std::string ext = _target.substr(pos);
    //std::cerr << "ext =" << ext << "\n";
    t_vector_str ext_list = _server.config.getValues(_target, "cgi_ext", {});
    if (find(ext_list.begin(), ext_list.end(), ext) == ext_list.end())
        return false;
    return true;
}

std::string Cgi::getResponse() {
    return _response;
}

std::string Cgi::readFromFd(int fd) {
    std::string result;
    char buffer[MAX_BUFFER_SIZE];
    ssize_t size;
    while ((size = read(fd, buffer, sizeof(buffer))) > 0) {
        //std::cout << "size=" << size <<"\n";
        result.append(buffer, size);
    }
    if (size < 0) {
        throw std::runtime_error("readFromFd: read error occured!");
    }
    return result;
}

int Cgi::_access(){
    _argv[0] = strdup(_env_map["SCRIPT_FILENAME"].c_str());
    if (!_argv[0])
        throw std::runtime_error("strdup error occurred!");
    _argv[1] = NULL;
    if (access(_argv[0], F_OK) != 0)
    {
        _status = 404; //"PATH NOT FOUND"
        return -1;
    }
    if (access(_argv[0], X_OK) != 0)
    {
        _status = 403; //"PERMISSION DENIED"
        return -1;
    }
    return 0;
}

void Cgi::_runChildCgi(){
    std::cout << "CGI: execve for ->" << _argv[0] << "<-\n";
    if (close(_fd_to_cgi[1]) == -1 || close(_fd_from_cgi[0]) == -1)
        throw std::runtime_error("close error occurred!");
    if (dup2(_fd_to_cgi[0], 0) == -1 || dup2(_fd_from_cgi[1], 1) == -1)
        throw std::runtime_error("dub2 error occurred!");
    if (close(_fd_to_cgi[0]) == -1 || close(_fd_from_cgi[1]) == -1)
            throw std::runtime_error("close error occurred!");
    execve(_argv[0], _argv, _envp);
    close(0);
    close(1);
    throw std::runtime_error("CGI:execve error occurred!");
}

bool Cgi::_wait(){
    time_t	start;
	int		waitResult;

	start = std::time(NULL);
	while (difftime(std::time(NULL), start) <= CGI_TIMEOUT)
	{
		waitResult = waitpid(_pid, &_status, WNOHANG);
		if (waitResult > 0)
			return true;
	}
	kill(_pid, SIGKILL);
	waitpid(_pid, &_status, 0);
    //_status = 504;
    std::cerr << "toooo long CGI was killed\n";
	return false;
}

void Cgi::runCmd(){
    if (_access() != 0)
        return ;
    //int _fd_from_cgi[2];
    //int _fd_to_cgi[2];
    if (pipe(_fd_to_cgi) == -1)
        throw std::runtime_error("pipe error occurred!");
    if (pipe(_fd_from_cgi) == -1){
        close(_fd_to_cgi[0]);
        close(_fd_to_cgi[1]);
        throw std::runtime_error("pipe error occurred!");
    }
    _pid = fork();
    if (_pid == -1){
        throw std::runtime_error("fork error occurred!");
    }
    else if (_pid == 0)
        _runChildCgi();
    if (close(_fd_to_cgi[0]) == -1 || close(_fd_from_cgi[1]) == -1)
        throw std::runtime_error("close error occurred!");
    //std::cout << "parent body: " << _body.c_str() << std::endl;
    write(_fd_to_cgi[1], _body.c_str(), _body.size());
    if (close(_fd_to_cgi[1]) == -1)
        throw std::runtime_error("close error occurred!");
    if (!_wait())
        _status = 504;
    else if ((_status & 0xff00) >> 8 != 0)
        _status = 500;
    else
    {
        _response = readFromFd(_fd_from_cgi[0]);
        _status = 0;
    }
    if (!_status && _response.empty())
    {
	    _response = "Internal Server Error\nContent-Type: text/plain\n";
        std::cout << "ERROR Empty response\n";
        _status = 500;
    }
    if (close(_fd_from_cgi[0]) == -1)
        throw std::runtime_error("close error occurred!");
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
    _envp = new char*[_env_map.size() + 1] {0};
    std::cout << "_env_map.size() + 1" << _env_map.size() << "\n";
    char* line_pnt;
    std::string line;
    int i = 0;
    for (auto& [key, value] : _env_map){
        line = key + "=" + value;
        //if (_envp[i] == nullptr)
        //    std::cout << "making envp[" << i << "] is NULL \n";
        //std::cout << "making envp[" << i << "] :" << line << "\n";  
        line_pnt = new char[line.size() + 1];
        std::strcpy(line_pnt, line.c_str());
        _envp[i] = line_pnt;
        ++i;
    }
    /*
    if (_envp[i] == nullptr)
        std::cout << "making envp[" << i << "] is NULL \n";
    else 
        _envp[i] = nullptr;
    */
}

void Cgi::setEnvMap(){
    _env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env_map["AUTH_TYPE"] = "basic";
    _env_map["REDIRECT_STATUS"] = "200";
    _env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env_map["SERVER_SOFTWARE"] = "Webserv_FAB/1.0";
    _env_map["DOCUMENT_ROOT"] = _server.config.getFirst("main","root","");

    _env_map["CONTENT_LENGTH"] = _request.getHeader("content-length");
	_env_map["CONTENT_TYPE"] = _request.getHeader("content-type");
    _env_map["HTTP_COOKIE"] = _request.getHeader("cookie");
    _env_map["HTTP_USER_AGENT"] = _request.getHeader("user-agent");
    _env_map["SERVER_NAME"] = _request.getHeader("Host");
    _env_map["REQUEST_METHOD"] = _request.get("method");
    size_t pos_query = _target.find('?');
    size_t pos_info = _target.rfind('/', pos_query);
    size_t pos_cgi = _target.find('/', 1);
    if (pos_query != std::string::npos)
        _env_map["QUERY_STRING"] = _target.substr(pos_query + 1);
    else
        _env_map["QUERY_STRING"] = "";
    if (pos_info != pos_cgi){
        _env_map["PATH_INFO"] = _target.substr(pos_info, pos_query - pos_info);
        _env_map["SCRIPT_FILENAME"] = _env_map["DOCUMENT_ROOT"] + _target.substr(0, pos_info);
    }
    else{
        _env_map["PATH_INFO"] = "";
        _env_map["SCRIPT_FILENAME"] = _env_map["DOCUMENT_ROOT"] + _target.substr(0, pos_query);
    }
    // SCRIPT_NAME  ???
    _env_map["PATH_TRANSLATED"] = _env_map["DOCUMENT_ROOT"] + _env_map["PATH_INFO"];
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

