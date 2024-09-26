/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/26 11:38:18 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


Client::Client(int fd, Server *server) : _fd(fd), _server(server), _request(nullptr), _res(nullptr), _responseSent(false) 
{
	_starttime = std::time(NULL);
	_totalBytesSent = 0;
	_cgireadpfd = nullptr;
	_force_closeconnection = false;
	resets = 0;
	_request = std::make_unique<Request>(_server);
	_res = std::make_unique<Response>(_fd, *_request, *_server);
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
	_cgireadpfd = assign._cgireadpfd;
	_starttime = assign._starttime;
	_totalBytesSent = assign._totalBytesSent;
	_force_closeconnection = assign._force_closeconnection;
	resets = assign.resets;
    return (*this);
}
Client::~Client() {}

bool Client::timeout(unsigned int timeout)
{
	// std::cout << "Uptime: " << difftime(std::time(NULL), starttime) << std::endl;
	if (difftime(std::time(NULL), _starttime) > timeout)
	{
		std::cout << "[INFO] Client timed out" << std::endl;
		if (!_res)
			_res = std::make_unique<Response>(_fd, *_request, *_server);
		_response = _res->getTimeOutErrorPage();
		_force_closeconnection = true;
		_starttime = std::time(NULL);
		return true;
	}
	return false;
}
int Client::get_socket_fd()
{
    return (_fd);
}

void Client::setcgiireadpfd(pollfd *pfd)
{
	_cgireadpfd = pfd;
}

int Client::get_cgi_fd()
{
	return _res->getCGIreadfd();
}

int Client::getCGIwritefd()
{
	return _res->getCGIwritefd();
}

int Client::readFromCGI()
{
	return _res->readfromCGI();
}

int Client::writeToCgi()
{
	return _res->writeToCgi();
}

void Client::resetForNextRequest()
{
	std::cout << "[INFO] Resetting client for next request: " << ++resets << std::endl;
	_request = std::make_unique<Request>(_server);
	_res = std::make_unique<Response>(_fd, *_request, *_server);
	_responseSent = false;
	_starttime = std::time(NULL);
	_totalBytesSent = 0;
	_response.clear();
	_force_closeconnection = false;

}

int Client::shouldCloseConnection()
{
	if (_request->getHeader("connection").compare("close") == 0) 
		return 1;
	if (_force_closeconnection)
		return 1;
	return 0;
}

bool Client::isRequestComplete()
{
	// std::cout << "[INFO] Request is body incomplete: " << _request->IsBodyIncomplete() << std::endl;
	// std::cout << "[INFO] Request bodyRaw size: " << _request->getBodyRawBytes().size() << std::endl;
	return !_request->IsBodyIncomplete() && _request->getBodyRawBytes().empty();
}

//return -1 = empty request
//Return 0 == Request fully read
//Return 1 == Body is not fully read
//Return 2 == CGI is waiting for body
//Return 3 == Headers not fully read
int Client::handle_request()
{ 
    int ret = _request->read(_fd);
	// std::cout << "[INFO] Request return: " << ret << std::endl;
	if (ret == -1)
		_force_closeconnection = 1;
	if (ret == 3 || ret == -1)
        return ret;
	// std::cout << "_responseSent: " << _responseSent << std::endl;
	if (_responseSent && _request->IsBodyIncomplete())
	{
		_request->getBodyRawBytes().clear();
		return 3;
	}
	if (_responseSent)
		return -1;
    _response = _res->run();
    return ret;
}


bool Client::responseReady()
{
    return !_response.empty();
}

int Client::send_cgi_response()
{
	ssize_t bytesSent;
	std::string &response = _res->getCgiResponse();
	if (response.empty())
		_force_closeconnection = true;
	if ((bytesSent = send(_fd, response.c_str(), std::min((size_t) 10000, response.size()), 0)) < 0)
	{
		std::cerr << "[ERROR] Send error: " << strerror(errno) << std::endl;
		_force_closeconnection = 1;
		return 1;
	}
	_res->getCgiResponse().erase(0, bytesSent);
	_totalBytesSent += bytesSent;
	std::cout << "Bytes sent: " << bytesSent << " TotalSent: " << _totalBytesSent << std::endl;
	// std::cout << "[INFO] Response Bytes sent: " << bytesSent << "/" << _totalBytesSent << std::endl;
	// std::cout << "is completed: " << isRequestComplete() << std::endl;
	if (!_force_closeconnection && _res->getCgiResponse().empty() && (!isRequestComplete() || (_cgireadpfd && _cgireadpfd->revents & (POLLIN | POLLHUP))))
	{
		// std::cout << "[INFO] CGI doesnt have more response in buffer, waiting for CGI read\n";
		return 2;
	}
	if (_force_closeconnection || (_cgireadpfd && _cgireadpfd->revents & POLLNVAL))
	{
		std::cout << "_force_closeconnection: " << _force_closeconnection << std::endl;
		
		// std::cout << "[INFO] CGI doesnt have more response in buffer and request is completed\n";
		return 1;
	}
	// if (!isRequestComplete())
	// 	std::cout << "[INFO] Request incomplete\n";
	// if (!_res->getCgiResponse().empty())
	// 	std::cout << "[INFO] CGI has more response buffer\n";
	return 0;
}

int Client::send_response()
{
    ssize_t bytesSent;
	if (_response.empty() && _request->isCGIflag())
	{
		int status = send_cgi_response();
		std::cout << "send_cgi_response status: " << status << std::endl;
		return status;
	}
    // std::cout << "---response----\n" << _response << "\n----END----" << std::endl;
	if ((bytesSent = send(_fd, _response.c_str(), std::min((size_t) 10000, _response.size()), 0)) < 0)
    {
        _force_closeconnection = 1;
        return 1;
    }
	_totalBytesSent += bytesSent;
	std::cout << "Bytes sent: " << bytesSent << " TotalSent: " << _totalBytesSent << std::endl;
	// std::cout << "[INFO] Response Bytes sent: " << bytesSent << "/" << _totalBytesSent << std::endl;
	if (bytesSent < (ssize_t)  _response.size())
	{
		// std::cout << "[INFO] Response will send the remaining on next loop\n";
		_response = _response.substr(bytesSent);
		return 0;
	}
    if (_res->hasMoreChunks())
    {
        // std::cout << "[INFO] Response has more chunks to send\n";
        _response = _res->getNextChunk();
        return 0;
    }
	if (!isRequestComplete())
	{
		// std::cout << "[INFO] Response is done and sent but Request incomplete\n";
		_responseSent = true;
		return 2;
	}
    return 1;
}

void Client::close_connection()
{
    std::cout << "[INFO] Closing connection" << std::endl;
    close(_fd);
    _server->remove_client(_fd);
}