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

void Response::run()
{
	std::string response;
	std::string method = _req.get("method");
	std::string target = _req.get("target");
	t_vector_str mtd_default = DEFAULT_METHOD;
	t_vector_str mtd = _srv.config.getValues(target, "limit_except", DEFAULT_METHOD);
	if (find(mtd_default.begin(), mtd_default.end(), method) == mtd_default.end())
		response = RESPONSE_501;
	else if (find(mtd.begin(), mtd.end(), method) == mtd.end())
		response = RESPONSE_405;
	else if (target.size() > 9 && target.substr(0, 9).compare("/cgi-bin/") == 0)
	{
		std::cout << "------- CGI ----------\n";
        Cgi cgi(_req, _srv);
		cgi.start();
		int status = cgi.getStatus();
		std::cerr << "status =" << status << "\n";
		if (status == 0)
		{
			response = cgi.getResponse();
			if (response.empty())
				response = RESPONSE_500;
			else
				response = STATUS_LINE_200 + response;
			// Add other validation of CGI response for status = 0;
			// Alternatevly we could put validation inside of Cgi class
		}
		else if (status == 403)
			response = RESPONSE_500;
		else if (status == 404)
			response = RESPONSE_500;
		else if (status == 500)
			response = RESPONSE_500;
		else if (status == 501)
			response = RESPONSE_500;
		else
			response = RESPONSE_500;
		//else if  (status == 502)
			//response = RESPONSE_502; //Bad Gateway
		//else if  (status == 504)
			//response = RESPONSE_504; // time out
		// to be extended:
		//else if (status == XXX)
		//	response = RESPONSE_XXX;
		std::cout << "------- END ----------" << std::endl;
	}
	else
	{
    	response = (method == "GET") ? get()
		: (method == "POST") ? post()
		: (method == "DELETE") ? deleteResp()
		: RESPONSE_501;
	}
	//std::cerr << "------- Err:Response ----------\n";
	std::cout << "------- Response ----------\n";
	std::cout << response << std::endl;
	std::cout << "------- END ---------------\n";
	if (send(_fd, response.c_str(), response.size(), 0) < 0)
		perror("Send error");
}

std::string Response::get()
{	
	std::string method = _req.get("method");
	std::string dir = _req.get("target");
	
	std::string response = "HTTP/1.1 200 OK\r\nContent-Type: text/plain";
	if (dir == "/" || dir == "/?Submit=Go+back")
		return load_index();
	if (dir == "/delete")
		return load_directory_listing();
	return (response);
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

std::string Response::load_index()
{
	std::ifstream file("www/index_cgi.html");
	if (!file.is_open())
		return ("HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\n\r\nError: index.html not found");
	
	std::stringstream buffer;
	if (_req.get("cookie").empty())
		buffer << createCookie();
	else
		_srv.saveCookieInfo(_req.getRef("cookie"));
	buffer << "\r\n";
	buffer << file.rdbuf();
	file.close();

	return ("HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n" + buffer.str());
}
std::string Response::load_directory_listing()
{
    DIR* dir;
    struct dirent* ent;
    std::string directoryPath = "uploads";
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
		buffer << "<form action=\"/\" method=\"get\"><input type=\"submit\" value=\"Go back\" name=\"Submit\" id=\"back\" /></form>";
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