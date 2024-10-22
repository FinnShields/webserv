/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/06 13:41:04 by apimikov          #+#    #+#             */
/*   Updated: 2024/09/30 11:47:39 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cgi.hpp"

#define DEBUG 0

Cgi::Cgi(const Cgi& other):
    _request(other._request),
    _server(other._server),
    _target(other._target),
    _index_virt(other._index_virt),
    _fd_from_cgi{ other._fd_from_cgi[0], other._fd_from_cgi[1]},
    _fd_to_cgi{ other._fd_to_cgi[0], other._fd_to_cgi[1]},
    _pid(0)
{
    _argv = new char*[4] {nullptr};
    _envp = nullptr;
}

Cgi::Cgi(Request& r, const Server& s, const size_t virt_index, std::string path):
    _request(r),
    _server(s),
    _target(r.get("target")),
    _index_virt(virt_index),
    _path(path),
    _fd_from_cgi{-1, -1},
    _fd_to_cgi{-1, -1},
    _pid(0)
{
    _argv = new char*[4] {nullptr};
    _envp = nullptr;
}

Cgi& Cgi::operator=(const Cgi&){
    return *this;
}

Cgi::~Cgi(){
    std::cerr << "[INFO] CGI destructor call\n";
    cleanEnv();
    for (int i = 0; _argv[i] != nullptr; i++)
        delete[] _argv[i];
    delete[](_argv);
	if (_pid > 0)
	{
		kill(_pid, SIGKILL);
		std::cerr << "[CGI Destructor] Child process killed\n";
	}
}

void Cgi::cleanEnv(){
    if (!_envp)
        return ;
    for (int i = 0; _envp[i] != nullptr; i++)
    {
        delete[] _envp[i];
        _envp[i] = nullptr;
    }
    delete[] _envp;
    _envp = nullptr;
}

int Cgi::getStatus(){
    return _status;
}

void Cgi::start(){
    if (DEBUG)
	{
		std::cout << "__________This is Cgi request._____________\n";
		std::cout << "Method =>" << _request.get("method") << "<=\n";
		std::cout << "Target =>" << _target << "<=\n";
	}
    setExtension();
    analizeTarget();
    setEnvMap();
    setEnv();
    if (_access() != 0)
        return ;
    if (!isImplemented())
    {
        _status = 501;
        return ;
    }
    try{
        runCmd();
    }
    catch (const std::runtime_error& e){
        std::cerr << "Caught Exception: " << e.what() << "\n";
        _status = 500;
    }
    catch (...) {
        std::cerr << "Caught unknown exception." << std::endl;
        _status = 500;
    }
}

void Cgi::runCmd(){
    if (pipe(_fd_to_cgi) == -1)
        throw std::runtime_error("pipe error occurred!");
    if (pipe(_fd_from_cgi) == -1){
        close(_fd_to_cgi[0]);
        close(_fd_to_cgi[1]);
        throw std::runtime_error("pipe error occurred!");
    }
    _pid = fork();
    if (_pid == -1)
        throw std::runtime_error("fork error occurred!");
    if (_pid == 0)
        _runChildCgi();
    if (close(_fd_to_cgi[0]) == -1 || close(_fd_from_cgi[1]) == -1)
        throw std::runtime_error("close error occurred!");
    // else if ((_status & 0xff00) >> 8 != 0)
    //     _status = 502;
    else
        _status = 0;
}

ssize_t Cgi::writeToPipe(const void *buf, size_t count)
{
	if (_fd_to_cgi[1] == -1)
        return 0;
    if (count > 0)
	{
		int bytesWritten = write(_fd_to_cgi[1], buf, std::min(count, (size_t)10000));
		std::cout << "[CGI] Wrote to pipe " << bytesWritten << " bytes" << std::endl;
		if (bytesWritten < 0)
        {
            close(_fd_to_cgi[1]);
            kill(_pid, SIGKILL);
            throw std::runtime_error("Write to cgi");
        }
        if (bytesWritten == 0)
            return 0;
		return bytesWritten;	
	}
	return 0;
}

std::string Cgi::readFromPipe()
{
    if (_fd_from_cgi[0] == -1)
        return "";
    char buffer[MAX_BUFFER_SIZE];
	ssize_t size = read(_fd_from_cgi[0], buffer, sizeof(buffer));
	std::cout << "[CGI] Read from pipe " << size << " bytes" << std::endl;
    if (size < 0)
    {
        kill(_pid, SIGKILL);
        return ("Status: 500 Internal Server Error");
    }
	if (size == 0)
	{
		_status = 200;
		return "";
	}
	if (waitpid(_pid, &_status, WNOHANG) != 0)
		_status = 200;
    return std::string(buffer, size);
}

int Cgi::get_pipereadfd()
{
	if (_status == 0 || _status == 200)
        return _fd_from_cgi[0]; 
    return -1;
}

int Cgi::get_writefd()
{
	return _fd_to_cgi[1];
}

void Cgi::setExtension()
{
    _pos_cgi = 0;
    _pos_dot = _path.find('.', _pos_cgi);
    if (_pos_dot == std::string::npos)
        return ;
    _pos_query = _path.find('?', _pos_dot);
    _pos_info = _path.find('/', _pos_dot);
    if (DEBUG){
        std::cerr << "_pos_dot =" << _pos_dot << "\n";
        std::cerr << "_pos_query =" << _pos_query << "\n";
        std::cerr << "_pos_info =" << _pos_info << "\n";
    }
    size_t ext_len = std::min(_path.size(), std::min(_pos_query,_pos_info)) - _pos_dot;
    _ext = _path.substr(_pos_dot, ext_len);
}

void Cgi::analizeTarget()
{
    
    if (_pos_info == std::string::npos && _pos_query == std::string::npos)
    {
        if (DEBUG)
			std::cout << "analizeTarget: no info_path, no query\n";
        _target_file_path = _path.substr(_pos_cgi);
        _target_query = "";
        _target_path_info = "";
    }
    else if (_pos_info == std::string::npos && _pos_query != std::string::npos)
    {
        if (DEBUG)
			std::cout << "analizeTarget: no info_path\n";
        _target_file_path = _path.substr(_pos_cgi, _pos_query - _pos_cgi);
        _target_path_info = "";
        _target_query = _path.substr(_pos_query + 1);
    }
    else if (_pos_info != std::string::npos && _pos_query == std::string::npos)
    {
        if (DEBUG)
			std::cout << "analizeTarget: no query\n";
        _target_file_path = _path.substr(_pos_cgi, _pos_info - _pos_cgi);
        _target_path_info = _path.substr(_pos_info);
        _target_query = "";
    }
    else if (_pos_info < _pos_query)
    {
        if (DEBUG)
			std::cout << "analizeTarget: info and query\n";
        _target_file_path = _path.substr(_pos_cgi, _pos_info - _pos_cgi);
        _target_path_info = _path.substr(_pos_info, _pos_query - _pos_info);
        _target_query = _path.substr(_pos_query + 1);
    }
    else
    {
        if (DEBUG)
			std::cout << "analizeTarget: ? in query\n";
        _target_file_path = _path.substr(_pos_cgi, _pos_query - _pos_cgi);
        _target_path_info = "";
        _target_query = _path.substr(_pos_query + 1);
    }
    if (DEBUG)
	{
		std::cout << " _ext=" << _ext << "\n";
		std::cout << " _target_file_path=" << _target_file_path << "\n";
		std::cout << " _target_path_info=" <<_target_path_info << "\n";
		std::cout << " _target_query=" << _target_query << "\n";
	}
	size_t pos_file = _target_file_path.rfind('/');
    if (pos_file == std::string::npos)
    {
        _target_file_name = _target_file_path;
        _target_foldername = "";
    }
    else
    {
        _target_file_name = _target_file_path.substr(pos_file + 1);
        _target_foldername = _target_file_path.substr(0, pos_file);
    }
    if (DEBUG)
	{	
		std::cout << " _target_foldername="<< _target_foldername << "\n";
    	std::cout << " _target_file_name=" << _target_file_name << "\n";
	}
}

bool Cgi::isImplemented()
{
    if (_pos_dot == std::string::npos)
    {
        std::cout << "CGI no extension\n";
        return false;
    }
    t_vector_str ext_list = _server.config.getValues(_index_virt, _target, "cgi_ext", {});
    auto it = find(ext_list.begin(), ext_list.end(), _ext);
    if (std::find(ext_list.begin(), ext_list.end(), _ext) == ext_list.end())
    {
        std::cout << "CGI extension is not implemented\n";
        return false;
    }
    _cgi_type = std::distance(ext_list.begin(), it);
    auto path_list = _server.config.getValues(_index_virt, _target, "cgi_path", {""});
    if (_cgi_type >= path_list.size())
    {
        std::cout << "CGI extension path is not implemented\n";
        return false;
    }
    _cgi_path = _server.config.getValues(_index_virt, _target, "cgi_path", {""})[_cgi_type];
    return true;
}

int Cgi::_access(){
	const char* file_path = _env_map["SCRIPT_FILENAME"].c_str();
    if (DEBUG)
			std::cout << "CGI: access for ->" << file_path << "<-\n";
    if (access(file_path, F_OK) != 0)
    {
        _status = 404; //"PATH NOT FOUND"
        return -1;
    }
    if (access(file_path, X_OK) != 0)
    {
        _status = 403;    //"PERMISSION DENIED"
        return -1;
    }
    return 0;
}

void Cgi::_runChildCgi(){
    if (_cgi_path == ".cgi")
    {
        _argv[0] = new char[_target_file_name.length() + 1];
        std::strcpy(_argv[0], _target_file_name.c_str());
        if (!_argv[0])
            throw std::runtime_error("strdup/strcpy error occurred!");
        _argv[1] = nullptr;
    }
    else
    {
        _argv[0] = new char[_cgi_path.length() + 1];
        std::strcpy(_argv[0], _cgi_path.c_str());
        if (!_argv[0])
            throw std::runtime_error("strdup/strcpy error occurred!");
        _argv[1] = new char[_target_file_name.length() + 1];
        std::strcpy(_argv[1], _target_file_name.c_str());
        if (!_argv[1])
            throw std::runtime_error("strdup/strcpy error occurred!");
    }
    _argv[2] = nullptr;
    if (DEBUG)
    {
        std::cerr << "CGI write fd: " << _fd_to_cgi[1] << " readfd: " << _fd_from_cgi[0] << std::endl;
        std::cerr << "CGI: cgi_path=" << _cgi_path << "<-" << std::endl;
        std::cerr << "CGI: execve for ->" << _argv[0] << "<- ->" << _argv[1] << "<- " << std::endl;
    }
    if (close(_fd_to_cgi[1]) == -1 || close(_fd_from_cgi[0]) == -1)
        throw std::runtime_error("close 	 error occurred!");
    if (dup2(_fd_to_cgi[0], 0) == -1 || dup2(_fd_from_cgi[1], 1) == -1)
        throw std::runtime_error("dub2 error occurred!");
    if (close(_fd_to_cgi[0]) == -1 || close(_fd_from_cgi[1]) == -1)
            throw std::runtime_error("close error occurred!");
	int max_fd = 1024;
	for (int i = 3; i < max_fd; i++)
		close(i);
	if (DEBUG)
	{
		std::cerr << "CGI: chdir to " << _target_foldername << std::endl;;
		std::cerr << "CGI: execve for ->" << _argv[0] << "<- ->" << _argv[1] << "<-" << std::endl;
	}
    std::string path = _target_foldername;
	if (DEBUG)
			 std::cerr << "CGI: chdir to " << path  << std::endl;
    chdir(path.c_str());
    execve(_argv[0], _argv, _envp);
    std::cerr << ("CGI: execve error occurred!") << std::endl;
	write(1, "Status: 500 Internal Server Error", 34);
	close(0);
    close(1);
	std::exit(0);
}

void Cgi::setEnv(){
    _envp = new char*[_env_map.size() + 1] {0};
    if (DEBUG)
		std::cout << "_env_map.size() + 1" << _env_map.size() << "\n";
    char* line_pnt;
    std::string line;
    int i = 0;
    for (auto& [key, value] : _env_map){
        line = key + "=" + value;
        line_pnt = new char[line.size() + 1] {0};
        std::strcpy(line_pnt, line.c_str());
        _envp[i] = line_pnt;
        ++i;
    }
}

void Cgi::setEnvMap(){
    _env_map["GATEWAY_INTERFACE"] = "CGI/1.1";
    _env_map["AUTH_TYPE"] = "basic";
    _env_map["REDIRECT_STATUS"] = "200";
    _env_map["SERVER_PROTOCOL"] = "HTTP/1.1";
	_env_map["SERVER_SOFTWARE"] = "Webserv_FAB/1.0";
    _env_map["DOCUMENT_ROOT"] = _server.config.getValues(_index_virt, _target, "root", {""})[0];
    if (_env_map["DOCUMENT_ROOT"].empty())
        _env_map["DOCUMENT_ROOT"] = _server.config.getFirst("main","root",""); // + "/cgi-bin";

    _env_map["CONTENT_LENGTH"] = _request.getHeader("content-length");
	_env_map["CONTENT_TYPE"] = _request.getHeader("content-type");
    _env_map["HTTP_COOKIE"] = _request.getHeader("cookie");
    _env_map["HTTP_USER_AGENT"] = _request.getHeader("user-agent");
    _env_map["SERVER_NAME"] = _request.getHeader("Host");
    _env_map["REQUEST_METHOD"] = _request.get("method");
    _env_map["SCRIPT_FILENAME"] = _target_file_path;
    _env_map["PATH_INFO_RFC"] = _target_path_info;
    _env_map["PATH_INFO"] = _target_file_path;
    _env_map["QUERY_STRING"] = _target_query;

    _env_map["PATH_TRANSLATED"] = _env_map["DOCUMENT_ROOT"] + _env_map["PATH_INFO"];
    size_t pos = _env_map["SERVER_NAME"].find(':');
    if (pos != std::string::npos)
        _env_map["SERVER_PORT"] = _env_map["SERVER_NAME"].substr(pos + 1);
    else
        _env_map["SERVER_PORT"] = "80";
    
    for (const auto& header : _request.getHeaders()) {
        std::string envName = "HTTP_" + header.first;
        std::replace(envName.begin(), envName.end(), '-', '_');
        std::transform(envName.begin(), envName.end(), envName.begin(), ::toupper);
        _env_map[envName.c_str()] = header.second.c_str();
    }

    for (auto it = _env_map.begin(); it != _env_map.end();) {
        if (it->second.empty()) {
            it = _env_map.erase(it);
        } else
            ++it;
    }
}  

