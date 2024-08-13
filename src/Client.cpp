/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/13 15:11:50 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


Client::Client(int fd) : _fd(fd), _request(NULL) {}
Client::Client(const Client &copy) : _fd(copy._fd), _request(copy._request), fileName(copy.fileName) {}
Client::~Client() 
{
}

Client &Client::operator=(const Client &assign)
{
	this->_fd = assign._fd;
    this->_request = assign._request;
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

int Client::handle_request(Server& srv)
{
    if (!_request)
    {
        std::cout << "new request" << std::endl;
        _request = new Request();
    }
    int ret = _request->read(_fd);
    // std::cout << "ret = " << ret << std::endl;
    // _request->display();
    if (ret == 0)
    {
        _response = Response(_fd, *_request, srv).run();
        delete _request;
        std::cout << "response is generated" << std::endl;
    }
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
    return 1;
}

void Client::close_connection(Server &srv)
{
    close(_fd);
    srv.remove_client(_fd);
}