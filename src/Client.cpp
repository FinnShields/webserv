/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 12:21:16 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/28 14:17:35 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"


Client::Client(int fd, Server *server) : _fd(fd), _server(server), _request(nullptr), _res(nullptr), _responseSent(false) {}
// Client::Client(const Client &copy) : _fd(copy._fd), _request(copy._request), _res(copy._res), _responseSent(copy._responseSent){}
Client::~Client() 
{
    std::cout << "[INFO] Client destructor" << std::endl;
    // if (_res)
    //     _res.reset();
    // if (_request)
    //     _request.reset();
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

//return -1 = empty request
//Return 0 == Request fully read
//Return 1 == Request is chunked (file)
//Return 3 == Headers not fully read
int Client::handle_request()
{
    if (!_request)
        _request = std::make_unique<Request>();
    int ret = _request->read(_fd);
    // std::cout << "request->read() returns: " << ret << std::endl;
    if (ret == 3 || ret == -1)
    {
        std::cout << ((ret == 3) ? "Headers are not fully read" : "Empty request") << std::endl;
        return ret;
    }
    if (!_res)
        _res = std::make_unique<Response>(_fd, *_request, *_server);
    _response = _res->run();
    if (_res->getcode() == 413)
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
    {
        perror("Send error");
        return 1;
    }
    std::cout << "[INFO] Response sent\n";
    if (_res->hasMoreChunks())
    {
        std::cout << "[INFO] More chunks to send\n";
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