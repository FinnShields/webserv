/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:05:15 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/07/04 06:37:04 by apimikov         ###   ########.fr       */
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
	_target = _req.get("target"); // maybe it should became private memeber

	if(!isMethodValid(method, response))
		;
	else if (_target.size() > 9 && _target.substr(0, 9).compare("/cgi-bin/") == 0)
		response = runCGI();
	else
    	response = 	(method == "GET") ? get() : 
					(method == "POST") ? post() :
					(method == "DELETE") ? deleteResp() : 
					RESPONSE_501;
	std::cout << "------- Response ----------\n" << response << "\n------- END ---------------\n";
	if (send(_fd, response.c_str(), response.size(), 0) < 0)
		perror("Send error");
}

//1. If target is regular file, loads it. 
//2. If index is on, load the index.
//3. If autoindex on, list files.
std::string Response::get()
{	
	_root = _srv.config.getValues(_target, "root", {""})[0];
	std::string loc = _srv.config.selectLocation(_target);
	loc = loc == "main" ? "" : loc;
	std::string target = _target.substr(loc.length());
	std::string path = _root + target;
	
	if (target.length() > 1 && std::filesystem::is_regular_file(path) && std::filesystem::exists(path))
		return load_file(path);
	
	_index = _srv.config.getValues(_target, "index", {""})[0];
	if (target.length() <=1 && _index.length() > 1) 
		return load_file(_root + "/" + _index);
	
	bool autoindex = _srv.config.getValues(_target, "autoindex", {"off"})[0] == "on";
	if (autoindex && std::filesystem::is_directory(path)) 
		return load_directory_listing(path);
	return (RESPONSE_404);
}

std::string Response::post()
{
	saveFile();
	// int status = saveFile();
	// return status == 500 ? "HTTP/1.1 500 Internal Server Error" :
	// 	// status == 400 ? "HTTP/1.1 400 Bad Request" :
	// 	"HTTP/1.1 204 No Content";
	return ("HTTP/1.1 204 No Content"); //No reloading
}

std::string Response::deleteResp()
{
	std::string target = _req.get("target");
	std::string dir = "uploads";

	replacePercent20withSpace(target);
	target = dir + target;
	if (deleteFile(target) == 204)
		return ("HTTP/1.1 204 No Content");
	return ("HTTP/1.1 404 Not Found");
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

std::string Response::runCGI()
{
	std::cout << "------- CGI ----------\n";
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

bool Response::isMethodValid(std::string &method, std::string &response)
{
	t_vector_str mtd_default = DEFAULT_METHOD;
	t_vector_str mtd_allowed = _srv.config.getValues(_target, "limit_except", DEFAULT_METHOD);
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

static bool isHtml(std::string fileName)
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

static std::string getFileName(std::string fileName)
{
	std::string filetype = fileName.substr(fileName.find_last_of('/') + 1);
	return filetype;
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
    DIR* dir;
    struct dirent* ent;
    std::stringstream buffer;

    if ((dir = opendir(directoryPath.c_str())) != NULL) 
	{
        buffer << "<html><head><title>Directory Listing</title></head><body>";
        buffer << "<h2>Directory Listing of /uploads</h2>";
        buffer << "<ul>";

        // List all the files and directories within directory
        while ((ent = readdir(dir)) != NULL) 
		{
			if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0)
			{
				buffer << "<li>" << ent->d_name
				<< "<input type=\"submit\" id=\"" << ent->d_name << "\" value=\"Delete\" />" 
				<< "<script>document.getElementById(\"" << ent->d_name << "\").addEventListener(\"click\", function() {var delReq = new XMLHttpRequest();"
				<< "delReq.open(\"DELETE\", \"/" << ent->d_name << "\", false);"
				<< "delReq.send(null); location.reload()});</script></li>";
			}
        }
        buffer << "</ul>";
		// buffer << "<form action=\"/\" method=\"get\"><input type=\"submit\" value=\"Go back\" name=\"Submit\" id=\"back\" /></form>";
        buffer << "</body></html>";

        closedir(dir);
    } 
	else 
	{
		std::cout << "couldnt open dir" << std::endl;
        return ("HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n\r\nError: Could not open directory");
    }

    return ("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n" + buffer.str());
}

bool Response::checkFileType(std::string& fileName)
{
	if (!fileName.compare(fileName.length() - 4, 4,  ".txt"))
		return true;
	else
		return false;

}

int Response::saveFile()
{
    if (_req.get("Content-Type").compare(0, 19, "multipart/form-data"))
        return 400;
    std::string boundary = _req.get("Content-Type").substr(31);
    std::string body = _req.get("body");
	if (body.empty())
		return 400;
    std::string fileName = "";
    std::string::iterator it = body.begin();
    it += body.find("filename") + 10;
	while (*it != '\"')
		fileName.append(1, *(it++));
	if (fileName.empty())
		return 400;
	if (checkFileType(fileName) == false)
		return 400;
    std::string directory = "uploads/";
    mkdir(directory.c_str(), 0777);
    fileName = directory + fileName;
    size_t start = body.find("\r\n\r\n") + 4;
    size_t len = body.find_last_of(boundary) - boundary.length() - 6 - start;
    std::string fileContent = body.substr(start, len);
    std::ofstream newFile(fileName);
    newFile << fileContent;
    newFile.close();
    _srv.setFileName(fileName);
	return 204;
}

int Response::deleteFile(std::string &file)
{
    if (std::remove(file.c_str()) < 0)
	{
        perror("remove");
		return 404;
	}
	return 204;
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