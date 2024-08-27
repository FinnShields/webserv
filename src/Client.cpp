/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/27 13:44:09 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


Client::Client(int fd) : _fd(fd), _request(NULL), _res(NULL), _responseSent(false) {}
Client::Client(const Client &copy) : _fd(copy._fd), _request(copy._request), _res(copy._res), _responseSent(copy._responseSent){}
Client::~Client() 
{
    std::cout << "[INFO] Client destructor" << std::endl;
    if (_res)
    {
        delete _res;
        _res = nullptr;
    }
    if (_request)
    {
        delete _request;
        _request = nullptr;
    }
}

Client &Client::operator=(const Client &assign)
{
	this->_fd = assign._fd;
    this->_request = assign._request;
    _res = assign._res;
    _responseSent = assign._responseSent;
	return (*this);
}

int Client::get_socket_fd()
{
    return (_fd);
} 

//return -1 = empty request
//Return 0 == Request fully read
//Return 1 == Request is chunked (file)
//Return 3 == Headers not fully read
int Client::handle_request(Server& srv)
{
    if (!_request)
        _request = new Request();
    int ret = _request->read(_fd);
    std::cout << "request->read() returns: " << ret << std::endl;
    if (ret == 3 || ret == -1)
    {
        std::cout << ((ret == 3) ? "Headers are not fully read" : "Empty request") << std::endl;
        return ret;
    }
    if (!_res)
        _res = new Response(_fd, *_request, srv);
    _response = _res->run();
    if (_response.substr(0, 12).compare("HTTP/1.1 413") == 0)
        ret = 0 ;
    return ret;
}
bool Client::responseReady()
{
    return !_response.empty();
}

int Client::send_response()
{
    _res->display();
    if (send(_fd, _response.c_str(), _response.size(), 0) < 0)
        perror("Send error");
    std::cout << "[INFO] Response sent\n";
    if (_res->hasMoreChunks())
    {
        std::cout << "[INFO] More chunks to send\n";
        _response = _res->getNextChunk();
        return 0;
    }
    if (_res)
    {
        delete _res;
        _res = nullptr;
    }
    return 1;
}

void Client::close_connection(Server &srv)
{
    close(_fd);
    srv.remove_client(_fd);
}