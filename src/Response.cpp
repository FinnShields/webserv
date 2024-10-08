/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:05:15 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/30 14:47:55 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"
#define DEBUG 0

const std::string Response::ABSOLUTE_PATH = Response::setAbsolutePath();

const std::string Response::setAbsolutePath()
{
    std::filesystem::path exePath = std::filesystem::read_symlink("/proc/self/exe");
    return exePath.parent_path().string() + '/';
}

Response::Response(int fd, Request &req, Server &srv, Client &client) : _fd(fd), _req(req), _srv(srv), _client(client), _file(0), _code(0)
{
}

Response::~Response() 
{
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
	if (_file == 1 || _file == 2)
	{
		std::remove(_fileName.c_str());
		std::cerr << "[FILE] Deleted due to interruption" << std::endl;
	}
}

/* File status
-1 = File failure
0 = no file / Upload succesfully.
1 = file created and closed
2 = file is opened in appending mode
3 = has more chunks to upload
*/
const std::string Response::run()
{
	if (_file != 0)
    	return _file == -1 ? getErrorPage(500) : appendfile();
    std::string method = _req.get("method");
    _target = _req.get("target");
	_index_virt = _srv.getVirtHostIndex(_req.get("host"));
    if (!checkRequestIsValid())
    {
        _req.getBodyRawBytes().clear();
		_client.set_closeconnection();
	    return appendHeadersAndBody(_response, true);
    }
	_response = _srv.config.getBestValues(_index_virt, _target, "return", {""})[0] != "" ? redirect() :
				isCGI() ? runCGI() :
				(method == "GET" || method == "HEAD") ? get() : 
				(method == "POST") ? post() :
				(method == "PUT") ? put() :
				(method == "DELETE") ? deleteResp() : 
				getErrorPage(501);
	return appendHeadersAndBody(_response, false);
}

bool Response::checkRequestIsValid()
{
    if(!isMethodValid(_req.get("method")))
        return false;
    if (_req.isBadRequest())
    {
        _req.getStatus() == 4 ? (_code = 414) : (_code = 400);
        _response = getErrorPage(_code);
        return false;
    }
    if (!supportHTTPversion())
    {
        _response = getErrorPage(_code);
        return false;
    }
    if(!validateContentLength())
    {
        _response = getErrorPage(413);
        return false;
    }
    return true;
}

bool Response::supportHTTPversion()
{
	std::string version = _req.get("version");
    if (!version.compare("HTTP/1.1")) {
        return true;
    }
	if (!version.compare("HTTP/0.9") || !version.compare("HTTP/1.0") ||
    !version.compare("HTTP/2") || !version.compare("HTTP/3"))
        _code = 505;
    else
        _code = 400;
    return false;
}
const std::string Response::invalidRequest(std::string response)
{
	_req.getBodyRawBytes().clear();
	return appendHeadersAndBody(response, true);
}

const std::string Response::appendHeadersAndBody(std::string &response, bool closeConnection)
{
	if (response.empty())
		return "";
	setCookie(response);
    if (closeConnection)
		response += "Connection: close\r\n";
	response += "\r\n";
	response += (_req.get("method").compare("HEAD") && !_body.empty()) ? _body : "";
	return response;
}

const std::string Response::redirect()
{
    t_vector_str redirect = _srv.config.getBestValues(_index_virt, _target, "return", {""});
    std::string response = redirect[0] == "301" ? "HTTP/1.1 301 Moved Permanently\r\n" :
                redirect[0] == "302" ? "HTTP/1.1 302 Found\r\n" :
                redirect[0] == "303" ? "HTTP/1.1 303 See Other\r\n" :
                redirect[0] == "307" ? "HTTP/1.1 307 Temporary Redirect\r\n" : "";
    if (response.empty())
        return getErrorPage(500);
    if (redirect.size() > 1 && !redirect[1].empty())
        response += "Location: " + redirect[1] + "\r\n";
    response += "Content-Length: 0\r\n"
                "Connection: close\r\n";
    return response;
}

const std::string Response::get()
{
    if (!_code) {
        _code = 200;
        _message = "OK";
    }
	std::string path = decodePath(getPath());	
	if (std::filesystem::is_regular_file(path) && std::filesystem::exists(path))
		return load_file(path);
	std::string _index = _srv.config.getBestValues(_index_virt, _target, "index", DEFAULT_INDEX)[0];
    path = path.back() == '/' ? path : path + '/';
	if (_index.length() > 1 && std::filesystem::is_regular_file(path + _index) && std::filesystem::exists(path + _index))
		return load_file(path + _index);
	bool autoindex = _srv.config.getBestValues(_index_virt, _target, "autoindex", {"off"})[0] == "on";
	if (autoindex && std::filesystem::is_directory(path)) 
		return load_directory_listing(path);
	return getErrorPage(404);
}

const std::string Response::post()
{
    try
    {
        if (_req.get("content-type").compare(0, 19, "multipart/form-data") == 0)
			_code = createFile(0);
		else
		{
			_code = 204;
			_req.getBodyRawBytes().clear();
		}
    }
    catch(const std::exception& e)
    {
        std::cerr << "[ERROR] " << e.what() << '\n';
        _file = -1;
        return getErrorPage(500);
    }
    _message = _code == 201 ? "Created" :
            _code == 204 ? "No Content" :
            "Unknown error";
    return _code == 201 ? STATUS_LINE_201 + contentLength(0) : 
			_code == 204 ? STATUS_LINE_204  : 
			getErrorPage(500);
}

const std::string Response::put()
{
    try
    {
        if (_req.get("content-type").compare(0, 19, "multipart/form-data") == 0)
			_code = createFile(0);
		else
		{
			_code = _code == 201 ? 201 : 
					_req.getBodyRawBytes().empty() ? 204 :
					201;
			_req.getBodyRawBytes().clear();
		}
    }
    catch(const std::exception& e)
    {
        std::cerr << "[ERROR] " << e.what() << '\n';
        _file = -1;
        return getErrorPage(500);
    }
    _message = _code == 201 ? "Created" :
            _code == 204 ? "No Content" :
            "Unknown error";
    return _code == 201 ? STATUS_LINE_201 + contentLength(0) : 
			_code == 204 ? STATUS_LINE_204 :
			getErrorPage(500);
}

const std::string Response::getErrorPage(int code)
{
    _code = code;
    _message = 	code == 400 ? "Bad Request" : 
				code == 405 ? "Not Allowed" :
                code == 413 ? "Content Too Large" :
                code == 500 ? "Internal Server Error" :
                code == 501 ? "Not Implemented" :
                code == 502 ? "Bad Gateway" :
                code == 504 ? "Gateway Timeout" :
                "Not Found";
	std::string error_page_path = _srv.config.getValues(_index_virt, _target, std::to_string(code), {"empty"})[0];
    std::string errorPath = (error_page_path != "empty" && !access(error_page_path.c_str(), R_OK)) ? error_page_path : 
                            "www/error_pages/" + std::to_string(code) + ".html";
	std::ifstream errorPage(errorPath);
	std::stringstream buffer;
	buffer << errorPage.rdbuf();
	_body = buffer.str();
	errorPage.close();
    std::string responseString = "HTTP/1.1 " + std::to_string(code) + " " + _message + "\r\nContent-Type: text/html\r\n";
    responseString += contentLength(_body.size());
	return (responseString);
}

const std::string Response::deleteResp()
{
    _code = 204;
    _message = "No content";
    std::string path = getPath();
	if (deleteFile(path) == 204)
		return (STATUS_LINE_204 + std::string("\r\n"));
	return (getErrorPage(404));
}

const std::string Response::runCGI()
{
	if (_cgi.get() == nullptr)
	{
		if (DEBUG)
			std::cerr << "------- CGI ----------\n";
        _cgi = std::make_unique<Cgi>(_req, _srv, _index_virt, getPath());
		_cgi->start();
		_code = _cgi->getStatus();
		if (DEBUG)
		{
			std::cerr << "CGI status = " << _code << "\n";
			std::cout << "------- END ----------" << std::endl;
		}
        if (_code == 0)
        {
            _cgi_response = STATUS_LINE_200;
            setCookie(_cgi_response);
        }
	}
	else
	{
		int haswritten = 0;
		for (pollfd &pfd : *_srv.get_fds())
			if (pfd.fd == _cgi->get_writefd())
			{
				pfd.events = POLLOUT;
				haswritten = 1;
				break;
			}
		if (!haswritten)
		{	std::cerr << "[INFO] Response could not find cgi pfd to set POLLOUT" << std::endl;
			if (_req.getBodyRawBytes().size() > 0)
				writeToCgi();
		}
	}
	return _cgi->getStatus() == 0 ? "" : getErrorPage(_cgi->getStatus());
}

int Response::writeToCgi()
{
	int bytesWritten;
	std::vector<char> &bodyraw = _req.getBodyRawBytes();
	if ((bytesWritten = _cgi->writeToPipe(_req.getBodyRawBytes().data(), _req.getBodyRawBytes().size())) > 0)
		bodyraw.erase(bodyraw.begin(), bodyraw.begin() + bytesWritten);
	return bodyraw.size();
}

size_t Response::readfromCGI()
{
	if (!_cgi)
        return 0;
    std::string tmp = _cgi->readFromPipe();
	if (tmp.find("Status: 500 Internal Server Error") != std::string::npos)
	{
		_cgi_response = invalidRequest(getErrorPage(500));
		return 0;
	}
	_cgi_response.append(tmp);
	return tmp.size();
}

const std::string Response::load_file(std::string filepath)
{
	_filestream_read.open(filepath, std::ios::binary);
	if (!_filestream_read.is_open())
		return (getErrorPage(404));
	
    std::string response = (!_req.get("Method").compare("POST")) ? STATUS_LINE_201 + ("Location: " + _fileName + "\r\n") : 
                            STATUS_LINE_200;
	if (isHtml(filepath))
    {
		std::stringstream buffer;
		buffer << _filestream_read.rdbuf();
        _filestream_read.close();
		_body = buffer.str();
		buffer.clear();
        response += "Content-Type: text/html\r\n";
        response += contentLength(_body.size());
    }
	else
	{
        response += "Transfer-Encoding: chunked\r\n";
        response += "Content-Type: application/octet-stream\r\n";
        if (_req.get("method").compare("HEAD"))
            _file = 3;
        else
            _filestream_read.close();
	}
	return response;
}

const std::string Response::load_directory_listing(std::string directoryPath)
{
    t_vector_str        directories;
    t_vector_str        files;
    std::string         res;
    std::ostringstream  oss;

    if (!load_directory_entries(directoryPath, directories, files))
        return (getErrorPage(403));

    if (_target == "/")
        _target = "";
    oss << "<html><head><title>Directory Listing</title></head><body>"
		<< "<h2>Directory Listing of " << htmlEscape(directoryPath) << "</h2><ul>"
    	<<"<li><a href=\"../\">..</a>";
    for (const auto& dirName : directories) 
        oss << "<li id=\"" << dirName << "\"><a href=\"" << _target << "/" << dirName << "\">" << dirName << "</a>"
                << "<button type=\"button\" onclick=\"deleteFile('" << dirName << "')\">Delete</button></li>";
    for (const auto& fileName : files)
        oss << "<li id=\"" << fileName << "\"><a href=\"" << _target << "/" << fileName << "\" download>" << fileName << "</a>"
                << "<button type=\"button\" onclick=\"deleteFile('" << fileName << "')\">Delete</button></li>";
    oss << "</ul><script>function deleteFile(fileName) {fetch('" << _target << "/' + encodeURIComponent(fileName), { method: 'DELETE' })\
            .then(response => { if (response.ok) document.getElementById(fileName).remove(); })\
            .catch(error => console.error('Error:', error));}</script></body></html>";
    _body = oss.str();
    res = _code == 201 ? STATUS_LINE_201 : STATUS_LINE_200 ;
    res += "Content-Type: text/html\r\n" + contentLength(_body.size());
    return (res);
}

void Response::setCookie(std::string &response)
{
    if (_client.getSessionID() != -1) {
        return ;
    }
	if (_req.get("cookie").empty() || _req.get("cookie").find("session-id") == std::string::npos) {
        response += createCookie();
    }
    else {
        std::string cookie = _req.getRef("cookie");
        _srv.saveCookieInfo(cookie);
        _client.setSessionID(std::atoi(cookie.c_str() + cookie.find("session-id=") + 11));
    }
}

std::string Response::createCookie()
{
	size_t newSessionId;

	srand((size_t) time(NULL));
	newSessionId = (size_t) rand();
	while (_srv.checkCookieExist(newSessionId))
		newSessionId = (size_t) rand();
	_srv.setNewCookie(newSessionId);
    _client.setSessionID(newSessionId);
	return ("Set-Cookie: session-id=" + std::to_string(newSessionId) + "\r\n");
}

bool Response::isCGI()
{
    return _req.isCGIflag();
}

bool Response::validateContentLength()
{
    const std::string max_body_size_str = _srv.config.getBestValues(_index_virt, _target, "client_max_body_size", {DEFAULT_MAX_BODY_SIZE})[0];
	if (max_body_size_str == "0")
        return true;
    size_t curr_body_size = _req.getBodyTotalSize();
	size_t max_body_size = std::stol(max_body_size_str);
	if (curr_body_size > max_body_size)
		return false;
    const std::string contentlength_str = _req.get("content-length");
    if (contentlength_str.empty())
        return true;
    _bodyMaxSize = max_body_size;
    size_t contentlength = std::stol(contentlength_str);
    return contentlength > max_body_size ? false : true;
}

std::string Response::decodePath(const std::string path)
{
	std::string decodedFileName = "";
	const char *pathptr = path.c_str();
    for (size_t i = 0; i < path.size(); i ++)
    {
        if (path[i] == '%')
        {
            decodedFileName.append(1, decodeChar(pathptr + i));
            i += 2;
        }
        else
            decodedFileName.append(1, path[i]);
    }
	return decodedFileName;
}

char Response::decodeChar(const char *ch)
{
    char    rtn = 0;
    char    first = *(ch + 1);
    char    last = *(ch + 2);

    if (first >= '2' && first <= '9')
        rtn = 16 * (first - '0');
    else if (first >= 'A' && first <= 'F')
        rtn = 16 * (first - 'A' + 10);
    if (last >= '0' && last <= '9')
        rtn += last - '0';
    else if (last >= 'A' && last <= 'F')
        rtn += last - 'A' + 10;
    return (rtn);
}

int Response::setFileName(std::vector<char> &bodyRaw, int type)
{
    std::cout << "[INFO] Setting file name" << std::endl;
	std::vector<char>::iterator it = bodyRaw.begin();
    size_t fileNamePos = findString(bodyRaw, "filename", 0);
    if (fileNamePos == std::string::npos)
        _fileName = "tempfile_" + std::to_string(std::time(nullptr));
	else
	{
		it += fileNamePos + 10;
		while (*it != '\"')
			_fileName.append(1, *(it++));
		if (_fileName.empty())
			_fileName = "tempfile_" + std::to_string(std::time(nullptr));
	}
    setDirectoryToFileName();
    if (type == 0)
        RenameIfFileExists();
    return 1;
}

void Response::setDirectoryToFileName()
{
    std::string directory = getPath();
    std::filesystem::create_directory(directory);
    if (directory.back() != '/')
        directory += '/';
    _fileName = directory + _fileName;
}

void Response::RenameIfFileExists()
{
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
}

void Response::checkOtherBoundary(std::vector<char> &bodyRaw, size_t &end, size_t offset)
{
    size_t otherBoundary = 0;
    while (otherBoundary != std::string::npos)
    {
        otherBoundary = findString(bodyRaw, _boundary, offset);
        if (otherBoundary == end)
            break ;
        if (otherBoundary != std::string::npos)
        {
            bodyRaw.erase(bodyRaw.begin() + otherBoundary, bodyRaw.begin() + otherBoundary + _boundary.size());
            end -= _boundary.size();
        }
    }
}

int Response::checkBodySize(std::vector<char> &bodyRaw)
{
    if (_fileCurrentSize + bodyRaw.size() >= _bodyMaxSize)
    {
        _file = 0;
        if (std::remove(_fileName.c_str()) < 0)
        {
            perror("delete");
            _code = 404;
            _message = "Not found";
            return (404);
        }
        _code = 413;
        _message = "Content too large";
        return (413);
    }
    return 0;
}

int Response::createFile(int type)
{
    std::vector<char> &bodyRaw = _req.getBodyRawBytes();
	if (bodyRaw.empty())
    {
		std::cout << "[INFO] No Body" << std::endl;
        return 204;
    }
    setFileName(bodyRaw, type);
    _boundary = _req.get("Content-Type").substr(31);
    size_t start = findString(bodyRaw, "\r\n\r\n", 0) + 4;
    size_t end = findString(bodyRaw, _boundary+"--", 0);
    checkOtherBoundary(bodyRaw, end, _boundary.size());
    end = end == std::string::npos ? bodyRaw.size() : end - 5;
    std::ofstream newFile(_fileName, std::ios::binary);
    if (!newFile.is_open())
    {
        deleteFile(_fileName);
        throw std::runtime_error("File is not opened");
    }
    newFile.write(bodyRaw.data() + start, end - start);
    newFile.close();
	bodyRaw.clear();
    _fileCurrentSize = end - start;
    _file = 1;
    std::cout << "[INFO] File created  " << _req.getBodyTotalSize() << "/" << _req.getHeader("content-length") << std::endl;
	if (_req.getBodyTotalSize() == std::stol(_req.getHeader("content-length")))
			_file = 0;
    return 201;
}

const std::string Response::appendfile()
{
    if (_file == 1)
    {
        _filestream.open(_fileName, std::ios::binary | std::ios::in | std::ios::out);
		_filestream.seekp(_fileCurrentSize );
        _file = 2;
    }
    std::vector<char> &bodyRaw = _req.getBodyRawBytes();
    if (int status = checkBodySize(bodyRaw) != 0)
        return invalidRequest(getErrorPage(status));
    size_t end = findString(bodyRaw, _boundary + "--", 0);
    checkOtherBoundary(bodyRaw, end, 0);
    end = end == std::string::npos ? bodyRaw.size() : end - 5;
	if (_filestream.is_open())
    {   
        _filestream.write(bodyRaw.data(), end);
        _fileCurrentSize += end;
		bodyRaw.clear();
        std::cout << "[INFO] File appended " << _req.getBodyTotalSize() << "/" << _req.getHeader("content-length") << std::endl;
		if (_req.getBodyTotalSize() == std::stol(_req.getHeader("content-length")))
			_file = 0;
        _response = STATUS_LINE_201;
		setCookie(_response);
		return _response + contentLength(0) + std::string("\r\n");
    }
    _file = -1;
    std::cerr << "[ERROR] File is not open" << std::endl;
    deleteFile(_fileName);
    return getErrorPage(500);
}

const std::string Response::getNextChunk()
{
    char buffer[MAX_BUFFER_SIZE] = {0};
    if (!_filestream_read.is_open())
    {
        _file = -1;
        std::cerr << "[ERROR] File is not open" << std::endl;
        return getErrorPage(500);
    }
    if (_filestream_read.eof() || _filestream_read.peek() == std::ifstream::traits_type::eof())
    {
        // std::cout << "[INFO] EOF reached" << std::endl;
        _file = 0;
        _filestream_read.close();
        return "0\r\n\r\n";
    }
    _filestream_read.read(buffer, MAX_BUFFER_SIZE);
    std::streamsize bytesRead = _filestream_read.gcount();
    
    if (_filestream_read.bad() || (_filestream_read.fail() && !_filestream_read.eof()))
    {
        _file = -1;
        std::cerr << "[ERROR] File read error" << std::endl;
        return getErrorPage(500);
    }
    std::stringstream hexStream;
    hexStream << std::hex << bytesRead;
    std::string chunk = hexStream.str() + "\r\n" + std::string(buffer, bytesRead) + "\r\n";
    return chunk;
}

bool Response::hasMoreChunks() const
{
    return _file == 3 ? true : false;
}

size_t  Response::findString(std::vector<char> bodyRaw, std::string str, size_t offset)
{
    if (offset >= bodyRaw.size())
        return (std::string::npos);
    for (size_t i = offset; i < bodyRaw.size(); i++)
    {
        for (size_t j = 0; j < str.size() && i + j < bodyRaw.size(); j++)
        {
            if (bodyRaw[i + j] != str[j])
                break ;
            if (j == str.size() - 1)
                return (i);
        }
    }
    return (std::string::npos);
}

int Response::deleteFile(const std::string &file)
{
    std::string decodedFileName = decodePath(file);
    // std::cout << "[INFO] Decoded name: \"" << decodedFileName << "\"" << std::endl;
    if (std::remove(decodedFileName.c_str()) < 0)
	{
        perror("remove");
		return 404;
	}
    std::cout << "[INFO] Deleted " << file << std::endl;
	return 204;
}

bool Response::isMethodValid(std::string method)
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
        return ABSOLUTE_PATH + _target.substr(1);
    if (alias.empty())
        return ABSOLUTE_PATH + root + _target;
    std::string loc = _srv.config.selectLocation(_target);
    loc = loc == "main" ? "/" : loc;
    std::string target = _target.substr(loc.length());
    return ABSOLUTE_PATH + alias + target;
}

bool Response::isHtml(const std::string fileName)
{
	if (fileName.length() > 5 && !fileName.compare(fileName.length() - 5, 5,  ".html"))
		return true;
	return false;
}

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

std::string Response::contentLength(size_t len)
{
	return "Content-Length: " + std::to_string(len) + "\r\n";
}

void Response::display() const
{
    if (!_code)
        return ;
    std::cout << "------- Response ----------" << std::endl;
    std::cout << "HTTP/1.1 " << _code << " " << _message << std::endl;
    std::cout << "---------------------------" << std::endl;
}

int Response::  getcode() const
{
    return _code;
}

const std::string Response::getTimeOutErrorPage()
{
    _target = _req.get("target");
    _index_virt = _srv.getVirtHostIndex(_req.get("host"));
	return invalidRequest(getErrorPage(504));
}

std::string &Response::getCgiResponse()
{
	return _cgi_response;
}

int Response::getCGIreadfd()
{
	return _cgi->get_pipereadfd();
}

int Response::getCGIwritefd()
{
	return _cgi->get_writefd();
}
