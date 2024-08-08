/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:48 by fshields          #+#    #+#             */
/*   Updated: 2024/08/08 10:32:05 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"		

Request::Request()
{
	_recvReturnTotal = 0;
}

Request::~Request()
{}

Request& Request::operator=(const Request& r)
{
	this->headers = r.headers;
	this->method = r.method;
	this->body = r.body;
	this->version = r.version;
	this->target = r.target;
    this->_recvReturnTotal = r._recvReturnTotal;
	return (*this);
}

Request::Request(const Request& r)
{
	*this = r;
}

bool Request::extractMethod(std::string& input)
{
	this->method = input.substr(0, input.find_first_of(' '));
	t_vector_str mtd_default = DEFAULT_METHOD;
	if (find(mtd_default.begin(), mtd_default.end(), method) == mtd_default.end())
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
	this->target = input.substr(start, end);
}

void Request::extractVersion(std::string& input)
{
	size_t start = input.find("HTTP");
	if (start == std::string::npos)
		perror("No http version");
	size_t end = 0;
	while (input.at(start + end) != '\r')
		end++;
	this->version = input.substr(start, end);
}

int	Request::read(int _fd)
{
	char buffer[MAX_BUFFER_SIZE] = {0};
    ssize_t recvReturn = recv(_fd, &buffer, MAX_BUFFER_SIZE, 0);
    if (recvReturn < 0)
        perror("Recv error");
    for (size_t i = 0; i < (size_t) recvReturn; i++)
        reqRaw.push_back(buffer[i]);
    _recvReturnTotal += recvReturn;
    std::cout << "recvReturn = " << recvReturn << "\nrecvTotal = " << _recvReturnTotal << std::endl;
    if (_recvReturnTotal == 0)
        return -1;
    this->parse(reqRaw);
	return headers["content-length"].empty() ? 0 : recvReturn;
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
		this->headers[first] = second;
		i += len + 1;
	}
}

void	Request::handleChunks(char *reqArray, size_t start)
{
	size_t contentLength = 0;
	size_t chunkLength = atoi(&reqArray[start]);
	std::vector<char> contentRawBytes;

	size_t i = start;
	while (chunkLength != 0)
	{
		contentLength += chunkLength;
		while (isdigit(reqArray[i]) || reqArray[i] == '\r' || reqArray[i] == '\n')
			i ++;
		while (reqArray[i] != '\r' && reqArray[i] != '\n')
			contentRawBytes.push_back(reqArray[i++]);
		while (!isdigit(reqArray[i]))
			i ++;
		chunkLength = atoi(&reqArray[i]);
	}
	this->headers["content-length"] = std::to_string(contentLength);
	this->bodyRawBytes = contentRawBytes;
	for (i = 0; i < bodyRawBytes.size(); i++)
		body.append(1, bodyRawBytes[i]);
}

void	Request::extractBody(std::vector<char> reqRaw)
{
	char *ch;
	char *reqArray = &reqRaw[0];

	ch = strstr(reqArray, "\r\n\r\n") + 4;
	if (!ch)
		return ;
	size_t start = ch - reqArray;
	if (!this->get("transfer-encoding").compare("chunked"))
		return this->handleChunks(reqArray, start);
	for (size_t i = 0; start + i < (size_t) _recvReturnTotal; i ++)
		bodyRawBytes.push_back(reqRaw[start + i]);
	for (size_t i = 0; i < bodyRawBytes.size(); i++)
		body.append(1, bodyRawBytes[i]);
}

void	Request::parse(std::vector<char> reqRaw)
{
	std::string	input = "";
	for (size_t i = 0; i < reqRaw.size(); i++)
		input.append(1, reqRaw[i]);
	if (!this->extractMethod(input))
		return ;
	this->extractTarget(input);
	this->extractVersion(input);
	this->extractHeaders(input);
	if (!this->get("transfer-encoding").empty())
		std::cout << "#### Received a " << this->get("transfer-encoding") << " request" << std::endl;
	this->extractBody(reqRaw);
}

const std::string	Request::get(std::string toGet)
{
	for (size_t i = 0; i < toGet.size(); i++)
		toGet[i] = std::tolower(toGet[i]);
	if (!toGet.compare("method"))
		return (this->method);
	if (!toGet.compare("target"))
		return (this->target);
	if (!toGet.compare("version"))
		return (this->version);
	if (!toGet.compare("body"))
		return (this->body);
	if (!this->headers.count(toGet))
		return ("");
	return (this->headers[toGet]);
}

const std::string	Request::getHeader(std::string toGet)
{
	for (size_t i = 0; i < toGet.size(); i++)
		toGet[i] = std::tolower(toGet[i]);
	if (!this->headers.count(toGet))
		return ("");
	return (this->headers[toGet]);
}

std::string& Request::getRef(std::string toGet)
{
	if (!toGet.compare("body"))
		return (this->body);
	if (!toGet.compare("method"))
		return (this->method);
	if (!toGet.compare("target"))
		return (this->target);
	if (!toGet.compare("version"))
		return (this->version);
	return (this->headers[toGet]);
}

std::vector<char> Request::getBodyRawBytes()
{
	return bodyRawBytes;
}

void	Request::display()
{
	std::cout << std::endl;
	std::cout << "---------------------" << std::endl;
	std::map<std::string, std::string>::iterator it;
	std::cout << "Method: " << this->method << std::endl;
	std::cout << "Request-target: " << this->target << std::endl;
	std::cout << "HTTP-version: " << this->version << std::endl;
	for(it = this->headers.begin(); it != this->headers.end(); it++)
		std::cout << it->first << ": " << it->second << std::endl;
	if (!this->get("content-type").compare(0, 19, "multipart/form-data"))
	{
		std::cout << "Body: <file data>" << std::endl << "---------------------" << std::endl;
		return ;
	}
	if (!this->body.empty())
		std::cout << "Body: " << this->body << std::endl;
	std::cout << "---------------------" << std::endl;
	std::cout << std::endl;
}