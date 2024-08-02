/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:05:15 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/02 06:00:26 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

Response::Response(int fd, Request &req, Server &srv) : _fd(fd), _req(req), _srv(srv) {}
Response::~Response() {}

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

void Response::run()
{
	std::string response;
	std::string method = _req.get("method");
	_target = _req.get("target");
	std::cout << "------VIRTUAL HOSTING tests-------\n";
	std::cout << "target=" << _target << "\n";
	std::cout << "host=" << _req.get("host") << "\n";
	std::cout << "port=" << _srv.get_port() << "\n";
	std::cout << "ip=" << _srv.get_ip_string() << "\n";
	std::cout << "ip=" << _srv.get_ip() << "\n";
	std::cout << "index=" << _srv.index << "\n";
	(_srv.config).realToVirtualHosts();
	std::cout << "----------------------------------\n";

	if(isMethodValid(method, response))
        response = 	(_target.size() > 9 && _target.substr(0, 9).compare("/cgi-bin/") == 0) ? runCGI() :
                    (method == "GET") ? get() : 
                    (method == "POST") ? post() :
                    (method == "DELETE") ? deleteResp() : 
                    RESPONSE_501;
	std::cout << "------- Response ----------\n" << response << "\n------- END ---------------\n";
	if (send(_fd, response.c_str(), response.size(), 0) < 0)
		perror("Send error");
}

std::string Response::get()
{	
	std::string path = getPath();
    std::cout << "PATH=" << path << std::endl;
	if (std::filesystem::is_regular_file(path) && std::filesystem::exists(path))
		return load_file(path);
	std::string _index = _srv.config.getValues(_target, "index", {""})[0];
    std::cout << "INDEXPATH=" << path + _index << std::endl;
	if (_index.length() > 1 && std::filesystem::is_regular_file(path + _index) && std::filesystem::exists(path + _index))
		return load_file(path + _index);
	bool autoindex = _srv.config.getValues(_target, "autoindex", {"off"})[0] == "on";
	if (autoindex && std::filesystem::is_directory(path)) 
		return load_directory_listing(path);
	return (RESPONSE_404);
}

std::string Response::post()
{
	if (!_req.get("content-type").compare(0, 19, "multipart/form-data"))
		saveFile();
	// int status = saveFile();
	// return status == 500 ? "HTTP/1.1 500 Internal Server Error" :
	// 	// status == 400 ? "HTTP/1.1 400 Bad Request" :
	// 	"HTTP/1.1 204 No Content";
	return ("HTTP/1.1 204 No Content"); //No reloading
}

std::string Response::deleteResp()
{
    std::string path = getPath();

	replacePercent20withSpace(path);
	// std::cout << "Deleting " << path << std::endl;
	if (deleteFile(path) == 204)
		return ("HTTP/1.1 204 No Content");
	return ("HTTP/1.1 404 Not Found");
}

std::string Response::runCGI()
{
	std::cout << "------- CGI ----------\n";
	if (_srv.config.selectLocation(_target) != "/cgi-bin")
	{
		std::cout << "CGI is not configured.\n";
		return RESPONSE_500;
	}
	Cgi cgi(_req, _srv);
	cgi.start();
	int status = cgi.getStatus();
	std::cout << "CGI status =" << status << "\n";
	std::cout << "------- END ----------" << std::endl;
	return status == 0 ? STATUS_LINE_200 + cgi.getResponse() :
			status == 403 ? RESPONSE_500 :
			status == 404 ? RESPONSE_500 :
			status == 500 ? RESPONSE_500 :
			status == 501 ? RESPONSE_501 : // cgi's ext is not implemented. 
			status == 502 ? RESPONSE_500 : //Bad Gateway
			status == 504 ? RESPONSE_500 : // time out
			RESPONSE_500;
}

std::string Response::load_file(std::string filepath)
{
	std::ifstream file(filepath);
	if (!file.is_open())
		return ("HTTP/1.1 404 Page Not Found\r\nContent-Type: text/plain\r\n\r\nError: " + filepath + " not found");
	
	std::stringstream buffer;
	if (_req.get("cookie").empty())
		buffer << createCookie();
	else
		_srv.saveCookieInfo(_req.getRef("cookie"));
	buffer << "\r\n";
	buffer << file.rdbuf();
	file.close();
	
	std::string response = STATUS_LINE_200;
	if (isHtml(filepath))
		response += "Content-Type: text/html\r\n";
	else
	{
		response += "Content-Type: */*\r\n";
		response += "Content-Disposition: attachment; filename=\"" + getFileName(filepath) + "\"\r\n";
	}
	response += buffer.str();
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

int Response::saveFile()
	{
	std::string boundary = _req.get("Content-Type").substr(31);
	std::string body = _req.get("body");
    std::vector<char> bodyRaw = _req.getBodyRawBytes();
	if (body.empty())
		return 400;
    std::string fileName = "";
    std::string::iterator it = body.begin();
    it += body.find("filename") + 10;
	while (*it != '\"')
		fileName.append(1, *(it++));
	if (fileName.empty())
		return 400;
    
    std::string directory = getPath() + "uploads/";
    mkdir(directory.c_str(), 0777);
    fileName = directory + fileName;
	std::cout << std::endl;
    size_t start = body.find("\r\n\r\n") + 4;
    size_t len = bodyRaw.size() - boundary.length() - 9 - start;
    std::fstream newFile;
	newFile.open(fileName, std::ios::binary | std::ios::out);
	for (size_t i = 0; i < len && i < bodyRaw.size(); i++) {
		newFile << bodyRaw[start + i];
	}
    newFile.close();
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

bool Response::isMethodValid(std::string &method, std::string &response)
{
	t_vector_str mtd_default = DEFAULT_METHOD;
	t_vector_str mtd_allowed = _srv.config.getValues(_target, "limit_except", DEFAULT_ALLOWED_METHOD);
	if (find(mtd_default.begin(), mtd_default.end(), method) == mtd_default.end())
	{
		std::cout << "no supported method="  << method << "\n";
		response = RESPONSE_501;
		return false;
	}
	if (find(mtd_allowed.begin(), mtd_allowed.end(), method) == mtd_allowed.end())
	{
		response = RESPONSE_405;
		return false;
	}
	return true;
}

std::string Response::getPath()
{
    std::string alias = _srv.config.getValues(_target, "alias", {""})[0];
    std::string root = _srv.config.getValues(_target, "root", {""})[0];
    
    if (alias.empty() && root.empty())
    {
        alias = _srv.config.get("main", "alias", 0);
        root = _srv.config.get("main", "root", 0);
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
