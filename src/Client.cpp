/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/30 14:38:39 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"

Client::Client(int fd, Server *server) : _fd(fd), _server(server), 
		_request(std::make_unique<Request>(_server)), 
		_res(std::make_unique<Response>(_fd, *_request, *_server, *this)),
		_response(""),
		_totalBytesSent(0),
		_starttime(std::time(NULL)),
		_cgireadpfd(nullptr),
		_responseSent(false),
		_force_closeconnection(false), 
		_resets(0),
		_sessionID(-1) 
{}

Client::~Client() {}

//return -1 = empty request
//Return 0 == Request fully read
//Return 1 == Body is not fully read
//Return 2 == CGI - Add CGI fds to pollfd and read response there.
//Return 3 == Headers not fully read / Chunk not fully
int Client::handle_request()
{
	_starttime = std::time(NULL);
    int ret = _request->read(_fd);
	std::cout << "Request return: " << ret << std::endl;
	if (ret == -1)
	{
		std::cerr << "[INFO] Client disconnected" << std::endl;
		_force_closeconnection = 1;
	}
	if (ret == 3 || ret == -1)
        return ret;
	if (_responseSent)
	{
		if (_request->IsBodyIncomplete())
		{
			_request->getBodyRawBytes().clear();
			return 3;
		}
		return -1;
	}
    _response = _res->run();
    return ret;
}

//Return 0 = Unfinsihed request or response
//Return 1 = Finished request and response
//Return 2 = Response finished but request is incomplete
int Client::send_response()
{
	_starttime = std::time(NULL);
    ssize_t bytesSent;
	if (_response.empty() && _request->isCGIflag())
		return send_cgi_response();
	if ((bytesSent = send(_fd, _response.c_str(), std::min((size_t) 10000, _response.size()), 0)) < 0)
        return (_force_closeconnection = 1);
	_totalBytesSent += bytesSent;
	std::cout << "Bytes sent: " << bytesSent << " TotalSent: " << _totalBytesSent << std::endl;
	if (bytesSent == 0)
		return (_force_closeconnection = 1);
	if (bytesSent < (ssize_t)  _response.size())
	{
		_response = _response.substr(bytesSent);
		return 0;
	}
    if (_res->hasMoreChunks())
    {
		std::cout << "[INFO] has more chunks" << std::endl;
        _response = _res->getNextChunk();
        return 0;
    }
	if (!isRequestComplete())
	{
		_responseSent = true;
		return 2;
	}
    return 1;
}

//Return 0 = Unfinsihed request or response
//Return 1 = Finished request and response
//Return 2 = Wait for CGI
int Client::send_cgi_response()
{
	ssize_t bytesSent;
	std::string &response = _res->getCgiResponse();
	if ((bytesSent = send(_fd, response.c_str(), std::min((size_t) 10000, response.size()), 0)) < 0)
	{
		std::cerr << "[ERROR] Send error" << std::endl;
		return (_force_closeconnection = 1);
	}
	if (bytesSent == 0)
		return (_force_closeconnection = 1);
	_res->getCgiResponse().erase(0, bytesSent);
	_totalBytesSent += bytesSent;
	std::cout << "Bytes sent: " << bytesSent << " TotalSent: " << _totalBytesSent << std::endl;
	if (!_force_closeconnection && _res->getCgiResponse().empty() && (!isRequestComplete() || (_cgireadpfd && _cgireadpfd->revents & (POLLIN | POLLHUP))))
		return 2;
	if (_force_closeconnection || (_cgireadpfd && _cgireadpfd->revents & POLLNVAL))
		return 1;
	return 0;
}

bool Client::timeout(unsigned int timeout)
{
	if (difftime(std::time(NULL), _starttime) > timeout)
	{
		std::cout << "[INFO] Client timed out" << std::endl;
		if (!_res)
			_res = std::make_unique<Response>(_fd, *_request, *_server, *this);
		_response = _res->getTimeOutErrorPage();
		_force_closeconnection = true;
		_starttime = std::time(NULL);
		return true;
	}
	return false;
}

void Client::resetForNextRequest()
{
	std::cout << "[INFO] Resetting client for next request: " << ++_resets << std::endl;
	_request = std::make_unique<Request>(_server);
	_res = std::make_unique<Response>(_fd, *_request, *_server, *this);
	_response.clear();
	_totalBytesSent = 0;
	_starttime = std::time(NULL);
	_cgireadpfd = nullptr;
	_responseSent = false;
	_force_closeconnection = false;
}

void Client::close_connection()
{
    std::cout << "[INFO] Closing connection" << std::endl;
    close(_fd);
    _server->remove_client(_fd);
}

int Client::shouldCloseConnection()
{
	if (_request->getHeader("connection").compare("close") == 0) 
		return 1;
	if (_force_closeconnection)
		return 1;
	return 0;
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
	_starttime = std::time(NULL);
	return _res->readfromCGI();
}

int Client::writeToCgi()
{
	_starttime = std::time(NULL);
	return _res->writeToCgi();
}

bool Client::isRequestComplete()
{
	return !_request->IsBodyIncomplete() && _request->getBodyRawBytes().empty();
}

int Client::getSessionID()
{
	return (this->_sessionID);
}

void Client::setSessionID(int id)
{
	this->_sessionID = id;
}
