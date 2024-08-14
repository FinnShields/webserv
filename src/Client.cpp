/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/14 14:22:56 by bsyvasal         ###   ########.fr       */
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

//return -1 = empty request
//Return 0 == Request fully read
//Return 1 == Request is chunked (file)
//Return 2 == Request chunks are fully read.
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
    try
    {
        std::cout << "res run" << std::endl;
        _response = _res->run();
    }
    catch (const std::string e)
    {
        std::cout << "caught response" << std::endl;
        _response = e;
        ret = 0;
    }
    if (ret == 2)
    {
        std::cout << "[INFO] File closed" << std::endl;
        _res->closefile();
        ret = 0;
    }
    if (ret <= 0)
        delete _request;
    return ret;
}
bool Client::responseReady()
{
    return !_response.empty();
}

int Client::send_response()
{
    std::cout << "------- Response ----------\n" << _response.substr(0, 50) << "\n------- END ---------------\n";
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