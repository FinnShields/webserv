/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/17 15:08:06 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


Client::Client(int fd, Server *server) : _fd(fd), _server(server), _request(nullptr), _res(nullptr), _responseSent(false), _isCGI(false) 
{
	starttime = std::time(NULL);
}
Client::Client(const Client &copy)
{
    *this = copy;
}

Client& Client::operator=(const Client &assign)
{
    _request = std::make_unique<Request>(*assign._request);
    _res = std::make_unique<Response>(*assign._res);

    *_request = *(assign._request);
    *_res = *(assign._res);
    _fd = assign._fd;
    _server = assign._server;
    _response = assign._response;
    _responseSent = assign._responseSent;
    return (*this);
}
Client::~Client() {}

bool Client::timeout(unsigned int timeout)
{
	// std::cout << "Uptime: " << difftime(std::time(NULL), starttime) << std::endl;
	if (difftime(std::time(NULL), starttime) > timeout)
	{
		std::cout << "[INFO] Client timed out" << std::endl;
		if (!_res)
			_res = std::make_unique<Response>(_fd, *_request, *_server);
		_response = _res->getTimeOutErrorPage();
		return true;
	}
	return false;
}
int Client::get_socket_fd()
{
    return (_fd);
}

int Client::get_cgi_fd()
{
	return _res->getCGIfd();
}

int Client::readFromCGI()
{
	_response = _res->readfromCGI();
	return (_response.size());
}

//return -1 = empty request
//Return 0 == Request fully read
//Return 1 == Body is not fully read
//Return 2 == Body is fully read, wait for CGI.
//Return 3 == Headers not fully read
int Client::handle_request()
{
    if (!_request)
        _request = std::make_unique<Request>(_server);
    int ret = _request->read(_fd);
	std::cout << "[INFO] Request return: " << ret << std::endl;
    if (ret == 3 || ret == -1)
    {
		std::cout << "[INFO] Request " << ((ret == 3) ? "has unread headers" : "failed/is empty") << std::endl;
        return ret;
    }
	if (ret == 2)
    {
		_isCGI = true;
    }
    if (!_res)
    {
        _res = std::make_unique<Response>(_fd, *_request, *_server);
    }
    _response = _res->run();
    if (_res->getcode() == 413 || (_isCGI && _res->getcode() != 0))
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

int Client::send_response()
{
    ssize_t bytesSent;
	// _res->display();
    //std::cout << "---response----\n" << _response << "\n----END----" << std::endl;
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