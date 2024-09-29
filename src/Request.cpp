/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:48 by fshields          #+#    #+#             */
/*   Updated: 2024/09/29 17:39:18 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"		
#define DEBUG 0

Request::Request(Server *srv) : _srv(srv)
{
	_recvReturnTotal = 0;
    _bodyTotalSize = 0;
    _status = 0;
	_currentChunkSize = -1;
	_chunkedReqComplete = true;
	_cgi_flag = false;
	_badrequest = false;
}

Request::~Request()
{}

//Return -1: Error or peer disconnected
//Return 0: No content-length or fully read
//Return 1: Body is not fully read
//Return 2: Body is fully read, but wait for CGI
//Return 3: Headers not fully recveived
//Status 0: No content-length or transfer-encoding
//Status 1: Content-length (Webkitforms)
//Status 2: Transfer-encoding (chunked)
int	Request::read(int _fd)
{
	char buffer[MAX_BUFFER_SIZE] = {0};
    ssize_t recvReturn = recv(_fd, &buffer, MAX_BUFFER_SIZE, 0);
	_recvReturnTotal += recvReturn;
    std::cout << " read bytes: " << recvReturn << " total bytes:" << _recvReturnTotal << std::endl;
	if (recvReturn < 0)
        throw std::runtime_error("Client read error: " + std::string(strerror(errno)));
	if (recvReturn == 0)
		return -1;
    for (size_t i = 0; i < (size_t) recvReturn; i++)
		_reqRaw.push_back(buffer[i]);
    if (_status == 0 && !isWholeHeader())
		return 3;
    _status == 1 ? resetBody()
		: _status == 2 ? chunkExtractBody(_reqRaw.data(), 0, _reqRaw.size())
		: _status == 3 ? handleChunks(_reqRaw.data(), 0, _reqRaw.size())
		: parse();
	if (_status == 3 || _status == -1)
		return _status;
    _status = !_headers["transfer-encoding"].empty() ? 2 :
        !_headers["content-length"].empty() ? 1 : 0;
	return _cgi_flag ? 2 : IsBodyIncomplete() ? 1 : 0;
}

void	Request::parse()
{
	std::string	input = "";
	for (size_t i = 0; i < _reqRaw.size(); i++)
		input.append(1, _reqRaw[i]);
	if (!extractMethod(input))
		return ;
	extractTarget(input);
	extractVersion(input);
	extractHeaders(input);
	if (!get("transfer-encoding").compare("chunked"))
		_chunkedReqComplete = false;
	extractBody();
	if (DEBUG)
    	display();
	setCGIflag();
}

bool Request::extractMethod(std::string& input)
{
	_method = input.substr(0, input.find_first_of(' '));
	t_vector_str mtd_default = DEFAULT_METHOD;
	if (find(mtd_default.begin(), mtd_default.end(), _method) == mtd_default.end())
		return (false);
	return (true);
}

void Request::extractTarget(std::string& input)
{
	size_t start;
	size_t end;

	start = input.find_first_of(' ') + 1;
	end = 0;
	while ((start + end) < input.size() && input.at(start + end) != ' ')
		end++;
	_target = input.substr(start, end);
}

void Request::extractVersion(std::string& input)
{
	size_t start = input.find("HTTP");
	if (start == std::string::npos)
	{
		_badrequest = true;	
		std::cerr << "[INFO] No http version" << std::endl;
		return ;
	}
	size_t end = 0;
	while (start + end < input.size() && input.at(start + end) != '\r')
		end++;
	_version = input.substr(start, end);
}


void	Request::extractHeaders(std::string& input)
{
	std::string	first;
	std::string second;
	size_t	len;

	if(input.find('\n') == std::string::npos)
		return ;
	for (size_t i = input.find('\n') + 1; i < input.size(); i++)
	{
		len = 0;
		while (input.size() > (i + len) && input.at(i + len) != ':')
			if (i + (++len) == input.size() || input.at(i + len) == '\n')
				return ;
		first = input.substr(i, len);
		for (size_t i = 0; i < first.size(); i++)
			first[i] = std::tolower(first[i]);
		i += len + 2;
		len = 0;
		while (input.size() > (i + len) && input.at(i + len) != '\r')
			len ++;
		second = input.substr(std::min((size_t) i, input.size()), len);
		_headers[first] = second;
		i += len + 1;
	}
}

void	Request::extractBody()
{
	const char headerEnd[] = "\r\n\r\n";
    auto it = std::search(_reqRaw.begin(), _reqRaw.end(), headerEnd, headerEnd + 4);
	if (it == _reqRaw.end())
		return ;
	it += 4;
	_reqRaw.erase(_reqRaw.begin(), it);
	if (!get("transfer-encoding").compare("chunked"))
		return handleChunks(_reqRaw.data(), 0, _reqRaw.size());
	for (size_t i = 0; i < (size_t) _reqRaw.size(); i++)
		_bodyRawBytes.push_back(_reqRaw[i]);
	_reqRaw.clear();
    _bodyTotalSize += _bodyRawBytes.size();
}

void    Request::resetBody()
{
    for (size_t i = 0; i < _reqRaw.size(); i++)
		_bodyRawBytes.push_back(_reqRaw[i]);
	_bodyTotalSize += _bodyRawBytes.size();
	_reqRaw.clear();
}

void	Request::handleChunks(char *reqArray, size_t i, size_t max_size)
{
	_status = 3;
	_chunkedReqComplete = false;
	if (i == max_size)
		return ;
	if (chunkExtractNumber(reqArray, i, max_size) < 0)
		return ;
	chunkExtractBody(reqArray, i, max_size);
}

int Request::chunkExtractNumber(char *reqArray, size_t &i, size_t max_size)
{
	size_t start = i;
	while (i < max_size && (isdigit(reqArray[i]) || (std::toupper(reqArray[i]) >= 'A' && std::toupper(reqArray[i])  <= 'E')))
		i++;
	if ((i < max_size && reqArray[i] != '\r') || (i + 1 < max_size && reqArray[i + 1] != '\n'))
	{
		std::cout << "[ERROR] Chunk number is invalid" << std::endl;
		_status = -1;
		return -1;
	}
	if (i + 1 >= max_size)
	{
		_reqRaw.erase(_reqRaw.begin(), _reqRaw.begin() + start);
		return -1;
	}
	_currentChunkSize = std::strtol(&reqArray[start], nullptr, 16);
	if (_currentChunkSize == 0)
	{
		if (i + 3 >= max_size)
		{
			_reqRaw.erase(_reqRaw.begin(), _reqRaw.begin() + start);
			return -1;
		}
		_chunkedReqComplete = true;
		_status = 2;
		return -1;
	}
	i += 2;
	return 1;
}

void Request::chunkExtractBody(char *reqArray, size_t i, size_t max_size)
{
	_status = 2;
	if (_currentChunkSize == -1)
		return handleChunks(_reqRaw.data(), 0, _reqRaw.size());
	while (i < max_size && _currentChunkBytes.size() < (size_t) _currentChunkSize + 2)
		_currentChunkBytes.push_back(reqArray[i++]);
	if (_currentChunkBytes.size() >= (size_t) _currentChunkSize + 2)
	{
		if (_currentChunkBytes.back() != '\n' || *(_currentChunkBytes.end() - 2) != '\r')
		{
			std::cout << "[ERROR] Chunk is invalid" << std::endl;
			_status = -1;
			return ;
		}
		std::cout << "[INFO] A chunk is validated" << std::endl;
		_bodyRawBytes.insert(_bodyRawBytes.end(), _currentChunkBytes.begin(), _currentChunkBytes.end() - 2);
		_bodyTotalSize += _currentChunkBytes.size() - 2;
		_currentChunkBytes.clear();
		_currentChunkSize = -1;
		_status = 3;
	}
	_reqRaw.erase(_reqRaw.begin(), _reqRaw.begin() + i);
	if (_reqRaw.size() > 0)
		handleChunks(reqArray, 0, _reqRaw.size());
}

bool Request::IsBodyIncomplete()
{
	if (_status == 0)
		return !isWholeHeader();
	if (_status == 1)
        return std::stol(_headers["content-length"]) > _bodyTotalSize ? 1 : 0;
	return !_chunkedReqComplete ? 1 : 0;
}

int Request::setCGIflag()
{
    if ((_target.size() > 9 && !_target.substr(0, 9).compare("/cgi-bin/"))){
		_cgi_flag = true;
		return true;
	}
	size_t index_virt = _srv->getVirtHostIndex(get("host"));
	auto value = _srv->config.getBestValues(index_virt, _target, "cgi_ext", {""});
	if (value[0] == "")
         return false;
    size_t pos_cgi = 1;   //  start from /
    size_t pos_dot = _target.find('.', pos_cgi);
    if (pos_dot == std::string::npos || pos_dot == _target.size() - 1)
        return false;
    size_t pos_query = _target.find('?', pos_dot);
    size_t pos_info = _target.find('/', pos_dot);
    size_t ext_len = std::min(_target.size(), std::min(pos_query,pos_info)) - pos_dot;
    std::string _ext = _target.substr(pos_dot, ext_len);
	if (find(value.begin(), value.end(), _ext) == value.end())
         return false;
	_cgi_flag = true;
    return true;
}

bool Request::isCGIflag(){
	return _cgi_flag;
}

bool Request::isWholeHeader()
{
 	const char headerEnd[] = "\r\n\r\n";
    auto it = std::search(_reqRaw.begin(), _reqRaw.end(), headerEnd, headerEnd + 4);
    return it != _reqRaw.end();
}

int Request::getStatus()
{
	return _status;
}

const std::string	Request::get(std::string toGet)
{
	for (size_t i = 0; i < toGet.size(); i++)
		toGet[i] = std::tolower(toGet[i]);
	if (!toGet.compare("method"))
		return (_method);
	if (!toGet.compare("target"))
		return (_target);
	if (!toGet.compare("version"))
		return (_version);
	if (!_headers.count(toGet))
		return ("");
	return (_headers[toGet]);
}

std::map<std::string, std::string> Request::getHeaders()
{
	return _headers;
}

const std::string	Request::getHeader(std::string toGet)
{
	for (size_t i = 0; i < toGet.size(); i++)
		toGet[i] = std::tolower(toGet[i]);
	if (!_headers.count(toGet))
		return ("");
	return (_headers[toGet]);
}

std::string& Request::getRef(std::string toGet)
{
	if (!toGet.compare("method"))
		return (_method);
	if (!toGet.compare("target"))
		return (_target);
	if (!toGet.compare("version"))
		return (_version);
	return (_headers[toGet]);
}

std::vector<char> &Request::getBodyRawBytes()
{
	return _bodyRawBytes;
}

ssize_t Request::getBodyTotalSize()
{
    return _bodyTotalSize;
}

void	Request::display()
{
	std::cout << std::endl;
	std::cout << "---------------------" << std::endl;
	std::cout << "Method: " << _method << std::endl;
	std::cout << "Request-target: " << _target << std::endl;
	std::cout << "HTTP-version: " << _version << std::endl;
	for(std::map<std::string, std::string>::iterator it = _headers.begin(); it != _headers.end(); it++)
		std::cout << it->first << ": " << it->second << std::endl;
	if (!get("content-type").compare(0, 19, "multipart/form-data"))
		std::cout << "_Body: <file data>" << std::endl;
	else if (!get("transfer-encoding").compare("chunked"))
		std::cout << "_Body: <chunked data>" << std::endl;
	else if (!_bodyRawBytes.empty())
	{
		std::cout << "_Body: ";
		for (size_t i = 0; i < _bodyRawBytes.size(); i++)
			std::cout << _bodyRawBytes[i];
		std::cout << std::endl;
	}
	std::cout << "---------------------\n" << std::endl;
}
bool Request::isBadRequest()
{
	return _badrequest;
}