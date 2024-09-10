/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:05:15 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/09 14:02:05 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

const std::string Response::ABSOLUTE_PATH = Response::setAbsolutePath();

const std::string Response::setAbsolutePath()
{
    std::filesystem::path exePath = std::filesystem::read_symlink("/proc/self/exe");
    return exePath.parent_path().string() + '/';
}

Response::Response(int fd, Request &req, Server &srv) : _fd(fd), _req(req), _srv(srv), _file(0), _code(0){}
Response::~Response() 
{
    std::cout << "[INFO] Response destructor" << std::endl;
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
-1 = File failure
0 = no file
1 = file created and closed
2 = file is opened in appending mode
3 = has more chunks to upload
*/

const std::string Response::getNextChunk()
{
    char buffer[MAX_BUFFER_SIZE] = {0};
    if (!_filestream_read.is_open())
    {
        _file = -1;
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
        _file = -1;
        std::cerr << "[ERROR] File read error" << std::endl;
        return getErrorPage(500);
    }
    std::cout << "[INFO] Chunk size = " << bytesRead << std::endl;
    std::stringstream hexStream;
    hexStream << std::hex << bytesRead;
    std::string chunk = hexStream.str() + "\r\n" + std::string(buffer, bytesRead) + "\r\n";
    return chunk;
}

bool Response::hasMoreChunks() const
{
    return _file == 3 ? true : false;
}

const std::string Response::run()
{
	if (_file != 0)
    	return _file == -1 ? getErrorPage(500) : appendfile();
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
        _response = _srv.config.getBestValues(_index_virt, _target, "return", {""})[0] != "" ? redirect() :
                    (_target.size() > 9 && _target.substr(0, 9).compare("/cgi-bin/") == 0) ? runCGI() :
                    (method == "GET") ? get() : 
                    (method == "POST") ? post() :
                    (method == "DELETE") ? deleteResp() : 
                    getErrorPage(501);
    return _response;
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
                "Connection: close\r\n\r\n";
    return response;
}

const std::string Response::get()
{
    if (!_code) {
        _code = 200;
        _message = "OK";
    }
	std::string path = getPath();
    std::cout << "PATH=" << path << std::endl;
	if (std::filesystem::is_regular_file(path) && std::filesystem::exists(path))
		return load_file(path);
	std::string _index = _srv.config.getBestValues(_index_virt, _target, "index", DEFAULT_INDEX)[0];
    path = path.back() == '/' ? path : path + '/';
	if (_index.length() > 1 && std::filesystem::is_regular_file(path + _index) && std::filesystem::exists(path + _index))
		return load_file(path + _index);
	bool autoindex = _srv.config.getBestValues(_index_virt, _target, "autoindex", {"off"})[0] == "on";
	if (autoindex && std::filesystem::is_directory(path)) 
		return load_directory_listing(path);
	return (getErrorPage(404));
}

const std::string Response::post()
{
    _code = 201;
    _message = "Created";
    if (!_req.get("content-type").compare(0, 19, "multipart/form-data"))
		try
        {
            saveFile();
        }
        catch(const std::exception& e)
        {
            std::cerr << "[ERROR] " << e.what() << '\n';
            _file = -1;
            return getErrorPage(500);
        }
    return get();
}

const std::string Response::getErrorPage(int code)
{
    _code = code;
    _message = code == 405 ? "Not Allowed" :
                code == 413 ? "Content Too Large" :
                code == 500 ? "Internal Server Error" :
                code == 501 ? "Not Implemented" :
                code == 502 ? "Bad Gateway" :
                code == 504 ? "Gateway Timeout" :
                "Not Found";
	std::string error_page_path = _srv.config.getValues(_index_virt, _target, std::to_string(code), {"empty"})[0];
    std::string errorPath = (error_page_path != "empty" && !access(error_page_path.c_str(), R_OK)) ? error_page_path : 
                            "www/error_pages/" + std::to_string(code) + ".html";
	// std::cout << "[TEST MSG, comment me] Error page for code " << code << " is ->" << errorPath << "<-\n"; 
	//t_vector_str pages = _srv.config.getValues(_index_virt, _target, "error_page", {"empty"});
	// for (size_t i = 0; i < pages.size() - 1; i++)
	// {
	// 	if (!pages[i].compare(std::to_string(code)) && !pages[i+1].empty() && 
	// 	!access(pages[i+1].c_str(), R_OK) && isHtml(pages[i+1]))
	// 		errorPath = pages[i+1];
	// }
	std::ifstream errorPage(errorPath);
	std::stringstream buffer;
	buffer << errorPage.rdbuf();
	errorPage.close();
    std::string responseString = "HTTP/1.1 " + std::to_string(code) + " " + _message + "\r\nContent-Type: text/html\r\n\r\n";
	responseString += buffer.str();
	return (responseString);
}

const std::string Response::deleteResp()
{
    _code = 204;
    _message = "No content";
    std::string path = getPath();
	if (deleteFile(path) == 204)
		return (STATUS_LINE_204);
	return (getErrorPage(404));
}

const std::string Response::runCGI()
{
	std::cerr << "------- CGI ----------\n";
	if (_srv.config.selectLocation(_target) != "/cgi-bin")
	{
		std::cerr << "CGI is not configured.\n";
		return (getErrorPage(500));
	}
	Cgi cgi(_req, _srv, _index_virt);
	cgi.start();
	int status = cgi.getStatus();
	std::cerr << "CGI status =" << status << "\n";
	std::cout << "------- END ----------" << std::endl;
	return status == 0 ? STATUS_LINE_200 + cgi.getResponse() :
			status == 403 ?  getErrorPage(403) :
			status == 404 ?  getErrorPage(404) :
			status == 500 ?  getErrorPage(500) :
			status == 501 ?  getErrorPage(501) : // cgi's ext is not implemented. 
			status == 502 ?  getErrorPage(502) : //Bad Gateway
			status == 504 ?  getErrorPage(504) : // time out
			getErrorPage(500);
}

const std::string Response::load_file(std::string filepath)
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
	
    std::string response = (!_req.get("Method").compare("POST")) ? STATUS_LINE_201 + ("Location: " + _fileName + "\r\n") : 
                            STATUS_LINE_200;
	if (isHtml(filepath))
    {
		buffer << _filestream_read.rdbuf();
        _filestream_read.close();
        response += "Content-Type: text/html\r\n";
	    response += buffer.str();
    }
	else
	{
        response += "Transfer-Encoding: chunked\r\n";
        response += "Content-Type: application/octet-stream\r\n\r\n";
        _file = 3;
	}
	return response;
}

const std::string Response::load_directory_listing(std::string directoryPath)
{
    std::stringstream   buffer;
    t_vector_str        directories;
    t_vector_str        files;

    if (!load_directory_entries(directoryPath, directories, files))
        return (getErrorPage(403));

    if (_target == "/")
        _target = "";
    buffer << "<html><head><title>Directory Listing</title></head><body>"
            << "<h2>Directory Listing of " << htmlEscape(directoryPath) << "</h2><ul>";
    buffer << "<li><a href=\"../\">..</a>";
    for (const auto& dirName : directories) 
        buffer << "<li id=\"" << dirName << "\"><a href=\"" << _target << "/" << dirName << "\">" << dirName << "</a>"
                << "<button type=\"button\" onclick=\"deleteFile('" << dirName << "')\">Delete</button></li>";
    for (const auto& fileName : files) 
        buffer << "<li id=\"" << fileName << "\"><a href=\"" << _target << "/" << fileName << "\" download>" << fileName << "</a>"
                << "<button type=\"button\" onclick=\"deleteFile('" << fileName << "')\">Delete</button></li>";
    buffer << "</ul><script>"
        << "function deleteFile(fileName) {"
        << "  fetch('" << _target << "/' + encodeURIComponent(fileName), { method: 'DELETE' })"
        << "    .then(response => { if (response.ok) document.getElementById(fileName).remove(); })"
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
    _bodyMaxSize = max_body_size;
    long body_size = std::stol(body_size_str);
    std::cout << "check_body_size: \nbody:" << body_size << "\nmax: " << max_body_size << std::endl;
    return body_size > max_body_size ? false : true;
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

int Response::setFileName(std::vector<char> &bodyRaw)
{
    std::vector<char>::iterator it = bodyRaw.begin();
    it += findString(bodyRaw, "filename", 0) + 10;
    while (*it != '\"')
        _fileName.append(1, *(it++));
    if (_fileName.empty())
        throw std::invalid_argument("No filename");
    setDirectoryToFileName();
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

int Response::saveFile()
{
    std::vector<char> bodyRaw = _req.getBodyRawBytes();
	if (bodyRaw.empty())
    {
		std::cout << "[INFO] No Body" << std::endl;
        return 400;
    }
    setFileName(bodyRaw);
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
    _fileCurrentSize = end - start;
    _file = 1;
    std::cout << "[INFO] File created  " << _req.getBodyTotalSize() << "/" << _req.getHeader("content-length") << std::endl;
    return 201;
}

const std::string Response::appendfile()
{
    if (_file == 1)
    {
        _filestream.open(_fileName, std::ios::binary | std::ios::app);
        _file = 2;
    }
    std::vector<char> bodyRaw = _req.getBodyRawBytes();
    if (int status = checkBodySize(bodyRaw) != 0)
        return getErrorPage(status);
    size_t end = findString(bodyRaw, _boundary + "--", 0);
    checkOtherBoundary(bodyRaw, end, 0);
    end = end == std::string::npos ? bodyRaw.size() : end - 5;
	if (_filestream.is_open())
    {
        _filestream.write(bodyRaw.data(), end);
        _fileCurrentSize += end;
        std::cout << "[INFO] File appended " << _req.getBodyTotalSize() << "/" << _req.getHeader("content-length") << std::endl;
        return get(); 
    }
    _file = -1;
    std::cerr << "[ERROR] File is not open" << std::endl;
    deleteFile(_fileName);
    return getErrorPage(500);
}

size_t  Response::findString(std::vector<char> bodyRaw, std::string str, size_t offset)
{
    for (size_t i = offset; i < bodyRaw.size(); i++)
    {
        for (size_t j = 0; j < str.size(); j++)
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
    std::string decodedFileName = "";
    for (size_t i = 0; i < file.size(); i ++)
    {
        if (file[i] == '%')
        {
            decodedFileName.append(1, decodeChar(&file.c_str()[i]));
            i += 2;
        }
        else
            decodedFileName.append(1, file[i]);
    }
    std::cout << "[INFO] Decoded name: \"" << decodedFileName << "\"" << std::endl;
    if (std::remove(decodedFileName.c_str()) < 0)
	{
        perror("remove");
		return 404;
	}
    std::cout << "[INFO] Deleted " << file << std::endl;
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

void Response::display() const
{
    if (!_code)
        return ;
    std::cout << "------- Response ----------" << std::endl;
    std::cout << "HTTP/1.1 " << _code << " " << _message << std::endl;
    std::cout << "---------------------------" << std::endl;
}

int Response::getcode() const
{
    return _code;
}
