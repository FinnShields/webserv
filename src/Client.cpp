/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/11 14:13:56 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


Client::Client(int fd, Server *server) : _fd(fd), _server(server), _request(nullptr), _res(nullptr), _responseSent(false), _isCGI(false) {}
// Client::Client(const Client &copy) : _fd(copy._fd), _request(copy._request), _res(copy._res), _responseSent(copy._responseSent){}
Client::~Client() 
{
    std::cout << "[INFO] Client destructor" << std::endl;
    // close_connection();
}

// Client &Client::operator=(const Client &assign)
// {
// 	this->_fd = assign._fd;
//     this->_request = assign._request;
//     _res = assign._res;
//     _responseSent = assign._responseSent;
// 	return (*this);
// }

int Client::get_socket_fd()
{
    return (_fd);
}

int Client::get_cgi_fd()
{
	return _res->getCGIfd();
}

//return -1 = empty request
//Return 0 == Request fully read
//Return 1 == Body is not fully read
//Return 2 == Body is fully read, wait for CGI.
//Return 3 == Headers not fully read
int Client::handle_request()
{
    if (!_request)
        _request = std::make_unique<Request>();
    int ret = _request->read(_fd);
    std::cout << "[INFO] request->read() returns: " << ret << std::endl;
    if (ret == 3 || ret == -1)
    {
        std::cout << "[INFO] Request " << ((ret == 3) ? "has unread headers" : "is empty") << std::endl;
        return ret;
    }
	if (ret == 2)
		_isCGI = true;
    if (!_res)
        _res = std::make_unique<Response>(_fd, *_request, *_server);
    _response = _res->run();
    if (_res->getcode() == 413 || (_isCGI && _res->getcode() != 200))
	{
        _isCGI = false;
		ret = 0;
	}
    return ret;
}
bool Client::responseReady()
{
    return !_response.empty();
}

#include <sys/ioctl.h> // For ioctl

int Client::send_response()
{
    ssize_t bytesSent;
	_res->display();
	if (_isCGI)
	{
		_response.append(_res->readfromCGI());
		std::cout << "[RESPONSE]\n" << _response << std::endl;
	}
    if ((bytesSent = send(_fd, _response.c_str(), std::min((size_t) 100000, _response.size()), 0)) < 0)
    {
        perror("Send error");
        return 1;
    }
	std::cout << "[INFO] Response Bytes sent: " << bytesSent << "/" << _response.size() << std::endl;
	if (bytesSent < (ssize_t) _response.size())
	{
		std::cout << "[INFO] Response will send the remaining on next loop\n";
		_response = _response.substr(bytesSent);
		return 0;
	}
    if (_res->hasMoreChunks())
    {
        std::cout << "[INFO] Response has more chunks to send\n";
        _response = _res->getNextChunk();
        return 0;
    }
    return 1;
}

void Client::close_connection()
{
    std::cout << "[INFO] Closing connection" << std::endl;
    close(_fd);
    _server->remove_client(_fd);
}