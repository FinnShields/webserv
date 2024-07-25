/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:48 by fshields          #+#    #+#             */
/*   Updated: 2024/07/11 06:24:20 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"		

Request::Request()
{}

Request::~Request()
{}

Request& Request::operator=(const Request& r)
{
	this->headers = r.headers;
	this->method = r.method;
	this->body = r.body;
	this->version = r.version;
	this->target = r.target;
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

void	Request::read(int _fd)
{
	char buffer[MAX_BUFFER_SIZE] = {0};
	recvReturn = recv(_fd, &buffer, MAX_BUFFER_SIZE, 0);
	if (recvReturn < 0)
		perror("Recv error");
	if (!*buffer)
	{
		std::cout << "Connection cancelled (empty buffer)" << std::endl;
		return ;
	}
	this->parse(buffer);
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

void	Request::handleChunks(std::string& input, size_t i)
{
	size_t contentLength = 0;
	size_t chunkLength = atoi(input.c_str() + i);
	std::string content;
	std::string::iterator it;

	it = input.begin();
	it += i;
	while (chunkLength != 0)
	{
		contentLength += chunkLength;
		while (isdigit(*it) || *it == '\r' || *it == '\n')
			it ++;
		while (*it != '\r' && *it != '\n')
			content.append(1, *(it++));
		while (!isdigit(*it))
			it ++;
		chunkLength = atoi(input.c_str() + std::distance(input.begin(), it));
	}
	this->headers["content-length"] = std::to_string(contentLength);
	this->body = content;
}

void	Request::extractBody(char *buffer)
{
	char *ch;

	ch = strstr(buffer, "\r\n\r\n") + 4;
	if (!ch)
		return ;
	size_t start = ch - buffer;
	for (size_t i = 0; start + i < (size_t) recvReturn; i ++)
		bodyRawBytes.push_back(buffer[start + i]);
	for (size_t i = 0; i < bodyRawBytes.size(); i++)
		body.append(1, bodyRawBytes[i]);
	// this->body = "";
	// i = input.find("\r\n\r\n");
	// if (i == std::string::npos)
	// 	return ;
	// i += 4;
	// if (i >= input.length())
	// 	return ;
	// if (!this->get("transfer-encoding").compare("chunked"))
	// 	return (this->handleChunks(input, i));
	// len = atoi(this->get("content-length").c_str());
	// for (size_t j = 0; j < len; j++)
	// 	this->body.append(1, input[i++]);
}

void	Request::parse(char *buffer)
{
	std::string	input(buffer);

	if (!this->extractMethod(input))
		return ;
	this->extractTarget(input);
	this->extractVersion(input);
	this->extractHeaders(input);
	this->extractBody(buffer);
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