/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:48 by fshields          #+#    #+#             */
/*   Updated: 2024/09/17 15:11:53 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"		

Request::Request(Server *srv) : _srv(srv)
{
	_recvReturnTotal = 0;
    _bodyTotalSize = 0;
    _status = 0;
	_chunkedReqComplete = true;
	_incompleteChunk = false;
	_bodyRawBytes.clear();
}

Request::~Request()
{}

Request& Request::operator=(const Request& r) 
{
	_srv = r._srv;
	_headers = r._headers;
	_method = r._method;
	_version = r._version;
	_target = r._target;
    _recvReturnTotal = r._recvReturnTotal;
	_chunkedReqComplete = r._chunkedReqComplete;
	_cgi_flag = r._cgi_flag;
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
	while ((start + end) < input.size() && input.at(start + end) != ' ')
		end++;
	_target = input.substr(start, end);
}

void Request::extractVersion(std::string& input)
{
	size_t start = input.find("HTTP");
	if (start == std::string::npos)
	{
		std::cerr << "[INFO] No http version" << std::endl;
		return ;
	}
	size_t end = 0;
	while (start + end < input.size() && input.at(start + end) != '\r')
		end++;
	_version = input.substr(start, end);
}

int Request::getStatus()
{
	return _status;
}
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
    std::cout << "recVReturn: " << recvReturn << std::endl;
	if (recvReturn <= 0)
	{
		std::cerr << (recvReturn == 0 ? "[INFO] Client disconnected" : "[ERROR] Recv error: ") << (recvReturn == -1 ? strerror(errno) : "") << std::endl;
        return -1;
	}
    for (size_t i = 0; i < (size_t) recvReturn; i++)
	{
		_reqRaw.push_back(buffer[i]);
		if (buffer[i] == '\r')
			std::cout << "\\r";
		else if (buffer[i] == '\n')
			std::cout << "\\n";
		else 
			std::cout << buffer[i];
	}
	std::cout << std::endl;
    if (_status == 0 && !isWholeHeader())
		return 3;
    _status == 1 ? resetBody() : _status == 2 ? moreChunks() : parse();
    _reqRaw.clear();
    _status = !_headers["transfer-encoding"].empty() ? 2 :
        !_headers["content-length"].empty() ? 1 : 0; 
	std::cout << "Request status: " << _status << std::endl;
	return IsBodyIncomplete() ? 1 : isCGI() ? 2 : 0;
}

bool Request::IsBodyIncomplete()
{
	if (_status == 1)
        return std::stol(_headers["content-length"]) > _bodyTotalSize ? 1 : 0;
	return !_chunkedReqComplete ? 1 : 0;
}
int Request::isCGI()
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
	if (_reqRaw.size() < 4)
		return false;
	for (auto it = _reqRaw.begin(); it + 3 != _reqRaw.end(); it++)
	{
		if (*it == '\r' && *(it + 1) == '\n' && *(it + 2) == '\r' && *(it + 3) == '\n')
			return true;
	}
	return false;
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
		// std::cout << "extract header 1 OK" << std::endl;
		first = input.substr(i, len);
		for (size_t i = 0; i < first.size(); i++)
			first[i] = std::tolower(first[i]);
		// std::cout << "extract header 2 OK" << std::endl;
		i += len + 2;
		len = 0;
		while (input.size() > (i + len) && input.at(i + len) != '\r')
			len ++;
		// std::cout << "extract header 3 OK" << input.size() << "---" << len << "---" << i << std::endl;
		second = input.substr(std::min((size_t) i, input.size()), len);
		//second[i+len+1] = 0;
		// std::cout << "extract header 4 OK" << std::endl;
		_headers[first] = second;
		i += len + 1;
	}
}

void	Request::handleChunks(char *reqArray, size_t start, size_t max_size)
{
	size_t chunkLength = std::strtol(&reqArray[start], nullptr, 16);
	std::vector<char> contentRawBytes;

	size_t i = start;
	while (chunkLength != 0 && i < max_size)
	{
		_bodyTotalSize += chunkLength;
		while (i < max_size && (isdigit(reqArray[i]) || (reqArray[i] >= 'A' && reqArray[i] <= 'E') ||
		reqArray[i] == '\r' || reqArray[i] == '\n'))
			i ++;
		if (i == max_size)
		{
			_incompleteChunk = true;
			break ;
		}
		while (i < max_size && reqArray[i] != '\r')
			contentRawBytes.push_back(reqArray[i++]);
		if (i == max_size)
		{
			_incompleteChunk = true;
			break ;
		}
		while (i < max_size && !isdigit(reqArray[i]) && !(reqArray[i] >= 'A' && reqArray[i] <= 'E'))
			i ++;
		if (i == max_size)
		{
			_incompleteChunk = true;
			break ;
		}
		chunkLength = std::strtol(&reqArray[i], nullptr, 16);
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
	if (!get("content-type").compare("multipart/form-data") || isCGI())
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
		if (i + 2 <= _reqRaw.size())
		{
			_incompleteChunk = false;
			handleChunks(&_reqRaw[0], i + 2, _reqRaw.size());
		}	
	}
	else
		handleChunks(&_reqRaw[0], 0, _reqRaw.size());
}

void	Request::extractBody()
{
	char *ch;
	char *reqArray = &_reqRaw[0];

	ch = strstr(reqArray, "\r\n\r\n");
	if (!ch)
		return ;
	ch += 4;
	size_t start = ch - reqArray;
	if (!get("transfer-encoding").compare("chunked"))
		return handleChunks(reqArray, start, _reqRaw.size());
	for (size_t i = 0; start + i < (size_t) _reqRaw.size(); i++)
		_bodyRawBytes.push_back(_reqRaw[start + i]);
    _bodyTotalSize += _bodyRawBytes.size();
}

void    Request::resetBody()
{
	if ((!get("content-type").compare(0, 19, "multipart/form-data")) || (isCGI()))
		_bodyRawBytes.clear();
    for (size_t i = 0; i < _reqRaw.size(); i++)
		_bodyRawBytes.push_back(_reqRaw[i]);
	_bodyTotalSize += _reqRaw.size();
}

void	Request::parse()
{
	// int i = 0;
	std::string	input = "";
	for (size_t i = 0; i < _reqRaw.size(); i++)
		input.append(1, _reqRaw[i]);
	if (!extractMethod(input))
		return ;
	// std::cout << "Extract " << i++ << " OK" << std::endl;
	extractTarget(input);
	// std::cout << "Extract " << i++ << " OK" << std::endl;
	extractVersion(input);
	// std::cout << "Extract " << i++ << " OK" << std::endl;
	extractHeaders(input);
	// std::cout << "Extract " << i++ << " OK" << std::endl;
	extractBody();
	// std::cout << "Extract " << i++ << " OK" << std::endl;
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
