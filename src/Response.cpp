/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:05:15 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/27 13:22:56 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(int fd, Request &req, Server &srv) : _fd(fd), _req(req), _srv(srv), _file(0){}
Response::~Response() 
{
    std::cout << "Response destructor" << std::endl;
    if (_filestream.is_open())
    {
        _filestream.close();
        std::cout << "[INFO] File closed" << std::endl;
    }
    if (_filestream_read.is_open())
    {
        _filestream_read.close();
        std::cout << "[INFO] File read closed" << std::endl;
    }
}

/* Supported status codes
200 OK
204 No Content
404 "PATH NOT FOUND" 
403 "PERMISSION DENIED"
405 method not allowed
500 unknown cgi problem 
501 method/extention is not implemented. Response (method) and Cgi (ext)
502 cgi execution problem 
504 time out

not implemented
418 "TEAPOT" 
*/

/* File status
0 = no file
1 = file created and closed
2 = file is opened in appending mode
3 = has more chunks to upload
*/

const std::string Response::appendfile()
{
	if (_file == 1)
    {
        _filestream.open(_fileName, std::ios::binary | std::ios::app);
        _file = 2;
    }
    std::vector<char> bodyRaw = _req.getBodyRawBytes();
    size_t end = findBoundary(bodyRaw, _boundary);
    end = end == std::string::npos ? bodyRaw.size() : end - 5;
	if (_filestream.is_open())
    {
        _filestream.write(bodyRaw.data(), end);
        std::cout << "[INFO] File appended" << std::endl;
    }
    return get(); 
}

size_t  Response::findBoundary(std::vector<char> bodyRaw, std::string boundary)
{
    for (size_t i = 0; i < bodyRaw.size(); i++)
    {
        for (size_t j = 0; j < boundary.size(); j++)
        {
            if (bodyRaw[i + j] != boundary[j])
                break ;
            if (j == boundary.size() - 1)
                return (i);
        }
    }
    return (std::string::npos);
}

const std::string Response::getNextChunk()
{
    char buffer[MAX_BUFFER_SIZE] = {0};
    if (!_filestream_read.is_open())
    {
        _file = 0;
        std::cerr << "[ERROR] File is not open" << std::endl;
        return getErrorPage(500);
    }
    if (_filestream_read.eof())
    {
        std::cout << "[INFO] EOF reached" << std::endl;
        _file = 0;
        _filestream_read.close();
        return "0\r\n\r\n";
    }
    _filestream_read.read(buffer, MAX_BUFFER_SIZE);
    std::streamsize bytesRead = _filestream_read.gcount();
    
    if (_filestream_read.bad() || (_filestream_read.fail() && !_filestream_read.eof()))
    {
        _file = 0;
        std::cerr << "[ERROR] File read error" << std::endl;
        return getErrorPage(500);
    }
    std::cout << "[INFO] Chunk size = " << bytesRead << std::endl;
    std::stringstream hexStream;
    hexStream << std::hex << bytesRead;
    std::string chunk = hexStream.str() + "\r\n" + std::string(buffer, bytesRead) + "\r\n";
    return chunk;
}

bool Response::hasMoreChunks()
{
    return _file == 3 ? true : false;
}

const std::string Response::run()
{
	if (_file == 1 || _file == 2)
    	return appendfile();
    std::string method = _req.get("method");
	_target = _req.get("target");
	_index_virt = _srv.getVirtHostIndex(_req.get("host"));
	std::cout << "[INFO] Request is addressed to server " << _srv.index << "\n";
	if (_srv.index != _index_virt)
		std::cout << "[INFO] Request is readdressed to virtual server " << _index_virt << "\n";
    if(!check_body_size()) 
    {
        return getErrorPage(413);
    }
	if(isMethodValid(method))
        _response = (_target.size() > 9 && _target.substr(0, 9).compare("/cgi-bin/") == 0) ? runCGI() :
                    (method == "GET") ? get() : 
                    (method == "POST") ? post() :
                    (method == "DELETE") ? deleteResp() : 
                    getErrorPage(501);
    return _response;
}

std::string Response::get()
{
	std::string path = getPath();
    std::cout << "PATH=" << path << std::endl;
	if (std::filesystem::is_regular_file(path) && std::filesystem::exists(path))
		return load_file(path);
	std::string _index = _srv.config.getBestValues(_index_virt, _target, "index", DEFAULT_INDEX)[0];
    std::cout << "INDEXPATH=" << path + _index << std::endl;
	if (_index.length() > 1 && std::filesystem::is_regular_file(path + _index) && std::filesystem::exists(path + _index))
		return load_file(path + _index);
	bool autoindex = _srv.config.getBestValues(_index_virt, _target, "autoindex", {"off"})[0] == "on";
	if (autoindex && std::filesystem::is_directory(path)) 
		return load_directory_listing(path);
	return (getErrorPage(404));
}

std::string Response::post()
{
    if (!_req.get("content-type").compare(0, 19, "multipart/form-data"))
		std::cout << "saveFile returns: " << saveFile() << std::endl;
    std::string path = getPath();
    if (std::filesystem::is_regular_file(path) && std::filesystem::exists(path))
		return load_file(path);
    std::string _index = _srv.config.getBestValues(_index_virt, _target, "index", DEFAULT_INDEX)[0];
    if (_index.length() > 1 && std::filesystem::is_regular_file(path + _index) && std::filesystem::exists(path + _index))
		return load_file(path + _index);
    return (getErrorPage(404));
}

std::string Response::getErrorPage(int code)
{
	std::string error_page_path = _srv.config.getValues(_index_virt, _target, std::to_string(code), {"empty"})[0];
	std::cout << "[TEST MSG, comment me] Error page for code " << code << " is ->" << error_page_path << "<-\n"; 
	t_vector_str pages = _srv.config.getValues(_index_virt, _target, "error_page", {"empty"});
	std::string responseString = code == 413 ? "HTTP/1.1 413 Payload Too Large\r\nContent-Type: text/html\r\n\r\n" :
            "HTTP/1.1 " + std::to_string(code) + " Not Found\r\nContent-Type: text/html\r\n\r\n";
	std::string errorPath = "www/error_pages/" + std::to_string(code) + ".html";
	for (size_t i = 0; i < pages.size() - 1; i++)
	{
		if (!pages[i].compare(std::to_string(code)) && !pages[i+1].empty() && 
		!access(pages[i+1].c_str(), R_OK) && isHtml(pages[i+1]))
			errorPath = pages[i+1];
	}
	std::ifstream errorPage(errorPath);
	std::stringstream buffer;
	buffer << errorPage.rdbuf();
	errorPage.close();
	responseString += buffer.str();
	return (responseString);
}

std::string Response::deleteResp()
{
    std::string path = getPath();

	replacePercent20withSpace(path);
	std::cout << "Deleting " << path << std::endl;
	if (deleteFile(path) == 204)
		return (STATUS_LINE_204);
	return (getErrorPage(404));
}

std::string Response::runCGI()
{
	std::cout << "------- CGI ----------\n";
	if (_srv.config.selectLocation(_target) != "/cgi-bin")
	{
		std::cout << "CGI is not configured.\n";
		return (getErrorPage(500));
	}
	Cgi cgi(_req, _srv, _index_virt);
	cgi.start();
	int status = cgi.getStatus();
	std::cout << "CGI status =" << status << "\n";
	std::cout << "------- END ----------" << std::endl;
	return status == 0 ? STATUS_LINE_200 + cgi.getResponse() :
			status == 403 ?  getErrorPage(500) :
			status == 404 ?  getErrorPage(500) :
			status == 500 ?  getErrorPage(500) :
			status == 501 ?  getErrorPage(501) : // cgi's ext is not implemented. 
			status == 502 ?  getErrorPage(500) : //Bad Gateway
			status == 504 ?  getErrorPage(500) : // time out
			getErrorPage(500);
}

std::string Response::load_file(std::string filepath)
{
	_filestream_read.open(filepath, std::ios::binary);
	if (!_filestream_read.is_open())
		return (getErrorPage(404));
	
	std::stringstream buffer;
	if (_req.get("cookie").empty())
		buffer << createCookie();
	else
		_srv.saveCookieInfo(_req.getRef("cookie"));
	buffer << "\r\n";
	
    std::string response;
    if (!_req.get("Method").compare("POST")) {
        response = STATUS_LINE_201;
        response += "Location: " + _fileName + "\r\n";
    }
    else {
        response = STATUS_LINE_200;
    }
	if (isHtml(filepath))
    {
		buffer << _filestream_read.rdbuf();
        _filestream_read.close();
        response += "Content-Type: text/html\r\n";
	    response += buffer.str();
    }
	else
	{
		// response += "Content-Type: */*\r\n";
		// response += "Content-Disposition: attachment; filename=\"" + getFileName(filepath) + "\"\r\n";
        response += "Transfer-Encoding: chunked\r\n";
        response += "Content-Type: application/octet-stream\r\n\r\n";
        _file = 3;
	}
	return response;
}

std::string Response::load_directory_listing(std::string directoryPath)
{
    std::stringstream   buffer;
    t_vector_str        directories;
    t_vector_str        files;

    if (!load_directory_entries(directoryPath, directories, files))
        return ("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nError: Could not open directory");

    buffer << "<html><head><title>Directory Listing</title></head><body>"
            << "<h2>Directory Listing of " << htmlEscape(directoryPath) << "</h2><ul>";
    buffer << "<li><a href=\"../\">..</a>";
    for (const auto& dirName : directories) 
        buffer << "<li><a href=\"" << _target << "/" << dirName << "\">" << dirName << "</a>"
                << "<button onclick=\"deleteFile('" << dirName << "')\">Delete</button></li>";
    for (const auto& fileName : files) 
        buffer << "<li><a href=\"" << _target << "/" << fileName << "\" download>" << fileName << "</a>"
                << "<button onclick=\"deleteFile('" << fileName << "')\">Delete</button></li>";
    buffer << "</ul><script>"
        << "function deleteFile(fileName) {"
        << "  fetch('" << _target << "/' + encodeURIComponent(fileName), { method: 'DELETE' })"
        << "    .then(response => { if (response.ok) location.reload(); })"
        << "    .catch(error => console.error('Error:', error));"
        << "}</script>"
        << "</body></html>";
    return ("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + buffer.str());
}

bool Response::check_body_size()
{
    const std::string max_body_size_str = _srv.config.getBestValues(_index_virt, _target, "client_max_body_size", {DEFAULT_MAX_BODY_SIZE})[0];
	if (max_body_size_str == "0")
        return true;
    const std::string body_size_str = _req.get("content-length");
    if (body_size_str.empty())
        return true;
    long max_body_size = std::stol(max_body_size_str);
    long body_size = std::stol(body_size_str);
    std::cout << "check_body_size: \nbody:" << body_size << "\nmax: " << max_body_size << std::endl;
    return body_size > max_body_size ? false : true;
}

int Response::saveFile()
	{
	_boundary = _req.get("Content-Type").substr(31);
	std::string body = _req.get("body");
    std::vector<char> bodyRaw = _req.getBodyRawBytes();
	if (body.empty())
    {
		std::cout << "No Body" << std::endl;
        return 400;
    }
    _fileName = "";
    std::string::iterator it = body.begin();
    it += body.find("filename") + 10;
	while (*it != '\"')
		_fileName.append(1, *(it++));
	if (_fileName.empty())
    {
		std::cout << "No file name" << std::endl;
        return 400;
    }
    
    std::string directory = getPath() + "uploads/";
    mkdir(directory.c_str(), 0777);
    _fileName = directory + _fileName;
    std::ofstream newFile;
    while (std::filesystem::exists(_fileName))
    {
        std::string filetype;
        size_t lastDot = _fileName.find_last_of('.');
        if (lastDot == std::string::npos)
            filetype = "";
        else
            filetype = _fileName.substr(lastDot);
        _fileName = _fileName.substr(0, lastDot) + "_copy" + filetype;
    }
	newFile.open(_fileName, std::ios::binary);
    std::cout << "[INFO] File created" << std::endl;
    
    size_t start = body.find("\r\n\r\n") + 4;
    size_t end = body.find(_boundary+"--");
    end = end == std::string::npos ? bodyRaw.size() : end - 5;
    newFile.write(bodyRaw.data() + start, end - start);
    newFile.close();
    _file = 1;
	return 204;
}

int Response::deleteFile(const std::string &file)
{
    if (std::remove(file.c_str()) < 0)
	{
        perror("remove");
		return 404;
	}
	return 204;
}

std::string Response::createCookie()
{
	size_t newSessionId;

	srand((size_t) time(NULL));
	newSessionId = (size_t) rand();
	while (_srv.checkCookieExist(newSessionId))
	{
		srand((size_t) time(NULL));
		newSessionId = (size_t) rand();
	}
	_srv.setNewCookie(newSessionId);
	return ("Set-Cookie: session-id=" + std::to_string(newSessionId) + "\r\n");
}

bool Response::isMethodValid(std::string &method)
{
	t_vector_str mtd_default = DEFAULT_METHOD;
	if (find(mtd_default.begin(), mtd_default.end(), method) == mtd_default.end())
	{
		std::cout << "no supported method="  << method << "\n";
		_response = getErrorPage(501);
		return false;
	}
	t_vector_str mtd_allowed = _srv.config.getBestValues(_index_virt, _target, "limit_except", DEFAULT_ALLOWED_METHOD);
	if (find(mtd_allowed.begin(), mtd_allowed.end(), method) == mtd_allowed.end())
	{
		_response = getErrorPage(405);
		return false;
	}
	return true;
}

std::string Response::getPath()
{
    std::string alias = _srv.config.getValues(_index_virt, _target, "alias", {""})[0];
    std::string root = _srv.config.getValues(_index_virt, _target, "root", {""})[0];
    
    if (alias.empty() && root.empty())
    {
        alias = _srv.config.getAll(_index_virt, "main", "alias", 0);
        root = _srv.config.getAll(_index_virt, "main", "root", 0);
    }
    if (alias.empty() && root.empty())
        return _target.substr(1);
    if (alias.empty())
        return root + _target;
    std::string loc = _srv.config.selectLocation(_target);
    loc = loc == "main" ? "/" : loc;
    std::string target = _target.substr(loc.length());
    return alias + target;
}

bool Response::isHtml(const std::string fileName)
{
	if (fileName.length() > 5 && !fileName.compare(fileName.length() - 5, 5,  ".html"))
		return true;
	return false;
}

// static std::string getFileType(std::string fileName)
// {
// 	std::string filetype = fileName.substr(fileName.find_first_of('.'));
// 	return filetype;
// }

std::string Response::getFileName(const std::string path)
{
	std::string filename = path.substr(path.find_last_of('/') + 1);
	return filename;
}

std::string Response::htmlEscape(const std::string& s) {
    std::string result;
    for (char c : s) {
        switch (c) {
            case '&': result += "&amp;"; break;
            case '<': result += "&lt;"; break;
            case '>': result += "&gt;"; break;
            case '\"': result += "&quot;"; break;
            case '\'': result += "&#39;"; break;
            default: result += c; break;
        }
    }
    return result;
}

bool Response::load_directory_entries(const std::string directoryPath, t_vector_str &directories, t_vector_str &files)
{
    DIR* dir;
    struct dirent* ent;

    if ((dir = opendir(directoryPath.c_str())) != NULL)
    {
         while ((ent = readdir(dir)) != NULL) 
        {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
            {
                std::string fileName = htmlEscape(ent->d_name);
                if (ent->d_type == DT_DIR) 
                    directories.push_back(fileName);
                else
                    files.push_back(fileName);
            }
        }
        closedir(dir);
        std::sort(directories.begin(), directories.end());
        std::sort(files.begin(), files.end());
        return true;
    }
    return false;
}

void Response::replacePercent20withSpace(std::string &str)
{
	size_t pos = str.find("%20");
	while (pos != std::string::npos)
	{
		str.replace(pos, 3, " ");
		pos = str.find("%20", pos + 1);
	}
}
