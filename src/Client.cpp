/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/19 14:07:14 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


Client::Client(int fd) : _fd(fd), _request(NULL), _res(NULL), _responseSent(false) {}
Client::Client(const Client &copy) : _fd(copy._fd), _request(copy._request), _res(copy._res), _responseSent(copy._responseSent){}
Client::~Client() 
{
    delete _request;
    delete _res;
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
int Client::handle_request(Server& srv)
{
    if (!_request)
        _request = new Request();
    int ret = _request->read(_fd);
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
    std::cout << "------- Response ----------\n" << _response << "\n------- END ---------------\n";
    if (send(_fd, _response.c_str(), _response.size(), 0) < 0)
        perror("Send error");
    return 1;
}

void Client::close_connection(Server &srv)
{
    close(_fd);
    srv.remove_client(_fd);
}