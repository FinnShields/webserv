/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:48 by fshields          #+#    #+#             */
/*   Updated: 2024/08/23 16:05:49 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"		

Request::Request()
{
	_recvReturnTotal = 0;
    _bodyTotalSize = 0;
    _status = 0;
	_chunkedReqComplete = true;
	_incompleteChunk = false;
}

Request::~Request()
{}

Request& Request::operator=(const Request& r)
{
	_headers = r._headers;
	_method = r._method;
	_version = r._version;
	_target = r._target;
    _recvReturnTotal = r._recvReturnTotal;
	_chunkedReqComplete = r._chunkedReqComplete;
	return (*this);
}

Request::Request(const Request& r)
{
	*this = r;
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
	while (input.at(start + end) != ' ')
		end++;
	_target = input.substr(start, end);
}

void Request::extractVersion(std::string& input)
{
	size_t start = input.find("HTTP");
	if (start == std::string::npos)
		perror("No http version");
	size_t end = 0;
	while (input.at(start + end) != '\r')
		end++;
	_version = input.substr(start, end);
}
//Return -1: Empty request
//Return 0: No content-length or fully read
//Return 1: Body is not fully read
//Return 3: Headers not fully recveived
//Status 0: No content-length or transfer-encoding
//Status 1: Content-length (Webkitforms)
//Status 2: Transfer-encoding (chunked)
int	Request::read(int _fd)
{
	char buffer[MAX_BUFFER_SIZE] = {0};
    ssize_t recvReturn = recv(_fd, &buffer, MAX_BUFFER_SIZE, 0);
    if (recvReturn < 0)
        perror("Recv error");
    for (size_t i = 0; i < (size_t) recvReturn; i++)
        _reqRaw.push_back(buffer[i]);
    _recvReturnTotal += recvReturn;
    if (_recvReturnTotal <= 0)
        return -1;
    _status == 1 ? resetBody() : _status == 2 ? moreChunks() : parse();
    if (_status == 0 && !isWholeHeader())
        return 3;
    _reqRaw.clear();
    _status = !_headers["transfer-encoding"].empty() ? 2 :
        !_headers["content-length"].empty() ? 1 : 0; 
    std::cout << "\nContent-length = " << _headers["content-length"] << "\nbodysize= " << _bodyRawBytes.size() << "\nbodyTotalSize=" << _bodyTotalSize << std::endl;
    if (_status == 1)
        return std::stol(_headers["content-length"]) > _bodyTotalSize ? 1 : 0;
    return _chunkedReqComplete ? 0 : !_chunkedReqComplete ? 1 : 0;
}

bool Request::isWholeHeader()
{
    char *ch = strstr(_reqRaw.data(), "\r\n\r\n");
    if (!ch)
        return false;
    return true;
}
void	Request::extractHeaders(std::string& input)
{
	std::string	first;
	std::string second;
	size_t	len;

	for (size_t i = input.find('\n') + 1; i < input.size(); i++)
	{
		len = 0;
		while (input.at(i + len) != ':')
			if (i + (++len) == input.size() || input.at(i + len) == '\n')
				return ;
		first = input.substr(i, len);
		for (size_t i = 0; i < first.size(); i++)
			first[i] = std::tolower(first[i]);
		i += len + 2;
		len = 0;
		while (input.at(i + len) != '\r' && i + len < input.size())
			len ++;
		second = input.substr(i, len);
		_headers[first] = second;
		i += len + 1;
	}
}

void	Request::handleChunks(char *reqArray, size_t start)
{
	size_t chunkLength = strtol(&reqArray[start], nullptr, 16);
	std::vector<char> contentRawBytes;

	size_t i = start;
	while (chunkLength != 0 && i < MAX_BUFFER_SIZE)
	{
		std::cout << "Size of Chunk: " << chunkLength << std::endl;
		_bodyTotalSize += chunkLength;
		while (i < MAX_BUFFER_SIZE && (isdigit(reqArray[i]) || (reqArray[i] >= 'A' && reqArray[i] <= 'E') ||
		reqArray[i] == '\r' || reqArray[i] == '\n'))
			i ++;
		if (i == MAX_BUFFER_SIZE)
		{
			_incompleteChunk = true;
			break ;
		}
		while (i < MAX_BUFFER_SIZE && reqArray[i] != '\r')
			contentRawBytes.push_back(reqArray[i++]);
		if (i == MAX_BUFFER_SIZE)
		{
			_incompleteChunk = true;
			break ;
		}
		while (i < MAX_BUFFER_SIZE && !isdigit(reqArray[i]) && !(reqArray[i] >= 'A' && reqArray[i] <= 'E'))
			i ++;
		if (i == MAX_BUFFER_SIZE)
		{
			_incompleteChunk = true;
			break ;
		}
		chunkLength = strtol(&reqArray[i], nullptr, 16);
	}
	for (i = 0; i < contentRawBytes.size(); i++)
		_bodyRawBytes.push_back(contentRawBytes[i]);
	if (chunkLength != 0)
		_chunkedReqComplete = false;
	else
		_chunkedReqComplete = true;
}

void	Request::moreChunks()
{
	_bodyRawBytes.clear();
	if (_incompleteChunk)
	{
		size_t i = 0;
		while (i < _reqRaw.size())
		{
			if (i < _reqRaw.size() - 1 && _reqRaw[i] != '\r' && _reqRaw[i + 1] != '\n')
				_bodyRawBytes.push_back(_reqRaw[i]);
			else if (i < _reqRaw.size()-1 && _reqRaw[i] == '\r' && _reqRaw[i + 1] == '\n')
				break ;
			i ++;
		}
		if (i != _reqRaw.size())
		{
			_incompleteChunk = false;
			handleChunks(&_reqRaw[0], i + 2);
		}	
	}
	else
	{
		handleChunks(&_reqRaw[0], 0);
	}
}

void	Request::extractBody()
{
	char *ch;
	char *reqArray = &_reqRaw[0];

	ch = strstr(reqArray, "\r\n\r\n") + 4;
	if (!ch)
		return ;
	size_t start = ch - reqArray;
	if (!get("transfer-encoding").compare("chunked"))
		return handleChunks(reqArray, start);
	for (size_t i = 0; start + i < (size_t) _reqRaw.size(); i++)
		_bodyRawBytes.push_back(_reqRaw[start + i]);
    _bodyTotalSize += _bodyRawBytes.size();
}

void    Request::resetBody()
{
    _bodyRawBytes.clear();
    for (size_t i = 0; i < (size_t) _reqRaw.size(); i++)
		_bodyRawBytes.push_back(_reqRaw[i]);
    _bodyTotalSize += _bodyRawBytes.size();
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
	extractBody();
    display();
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

std::vector<char> Request::getBodyRawBytes()
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
	std::map<std::string, std::string>::iterator it;
	std::cout << "Method: " << _method << std::endl;
	std::cout << "Request-target: " << _target << std::endl;
	std::cout << "HTTP-version: " << _version << std::endl;
	for(it = _headers.begin(); it != _headers.end(); it++)
		std::cout << it->first << ": " << it->second << std::endl;
	if (!get("content-type").compare(0, 19, "multipart/form-data"))
	{
		std::cout << "_Body: <file data>" << std::endl << "---------------------" << std::endl;
		return ;
	}
	if (!_bodyRawBytes.empty())
	{
		std::cout << "_Body: ";
		for (size_t i = 0; i < _bodyRawBytes.size(); i++)
			std::cout << _bodyRawBytes[i];
		std::cout << std::endl;
	}
	std::cout << "---------------------" << std::endl;
	std::cout << std::endl;
}