/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/06/14 17:52:14 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


Client::Client(int fd) : _fd(fd) {}
Client::Client(const Client &copy) : _fd(copy._fd), fileName(copy.fileName) {}
Client::~Client() {};
Client &Client::operator=(const Client &assign)
{
	this->_fd = assign._fd;
    this->fileName = assign.fileName;
	return (*this);
}

int Client::get_socket_fd()
{
    return (_fd);
}

std::string& Client::get_fileName()
{
    return (this->fileName);
}

void Client::saveFile(Request& request, Server& srv)
{
    if (request.get("Content-Type").compare(0, 19, "multipart/form-data"))
        return ;
    std::string boundary = request.get("Content-Type").substr(31);
    std::string body = request.get("body");
    std::string fileName = "";
    std::string::iterator it = body.begin();
    it += body.find("filename") + 10;
    while (*it != '\"')
        fileName.append(1, *(it++));
    if (fileName.empty())
        return ;
    size_t start = body.find("\r\n\r\n") + 4;
    size_t len = body.find_last_of(boundary) - boundary.length() - 6 - start;
    std::string fileContent = body.substr(start, len);
    std::ofstream newFile(fileName);
    newFile << fileContent;
    newFile.close();
    srv.setFileName(fileName);
}

void Client::deleteFile(Server& srv)
{
    if (srv.getFileName().empty())
        return ;
    if (std::remove(srv.getFileName().c_str()) < 0)
        perror("remove");
    srv.clearFileName();
}

void Client::handle_request(Server& srv)
{
	Request request;
    request.read(_fd);
    request.display();
    if (!request.get("method").compare("POST"))
    {
        this->saveFile(request, srv);
    }
	Response resp(_fd, request, srv);
	resp.run();
    if (!request.get("method").compare("DELETE"))
    {
        this->deleteFile(srv);
    }
}

void Client::close_connection(Server &srv)
{
    close(_fd);
    srv.remove_client(_fd);
}