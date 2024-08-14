/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/14 13:12:23 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


Client::Client(int fd) : _fd(fd), _request(NULL), _res(NULL) {}
Client::Client(const Client &copy) : _fd(copy._fd), _request(copy._request), _res(copy._res){}
Client::~Client() 
{
}

Client &Client::operator=(const Client &assign)
{
	this->_fd = assign._fd;
    this->_request = assign._request;
    _res = assign._res;
	return (*this);
}

int Client::get_socket_fd()
{
    return (_fd);
} 

int Client::handle_request(Server& srv)
{
    if (!_request)
    {
        std::cout << "new request" << std::endl;
        _request = new Request();
    }
    int ret = _request->read(_fd);
    if (!_res)
        _res = new Response(_fd, *_request, srv);
    _response = _res->run();
    if (ret == 2)
    {
        _res->closefile();
        ret = 0;
    }
    if (ret == 0)
        delete _request;
    return ret;
}
bool Client::responseReady()
{
    return !_response.empty();
}

int Client::send_response()
{
    if (send(_fd, _response.c_str(), _response.size(), 0) < 0)
        perror("Send error");
    delete _res;
    return 1;
}

void Client::close_connection(Server &srv)
{
    close(_fd);
    srv.remove_client(_fd);
}