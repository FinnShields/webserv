/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fshields <fshields@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:48 by fshields          #+#    #+#             */
/*   Updated: 2024/06/18 11:07:03 by fshields         ###   ########.fr       */
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
	return (*this);
}

Request::Request(const Request& r)
{
	*this = r;
}

void Request::extractMethod(std::string& input)
{
	this->method = input.substr(0, input.find_first_of(' '));
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
	size_t end = 0;
	while (input.at(start + end) != '\r')
		end++;
	this->version = input.substr(start, end);
}

void	Request::read(int _fd)
{
	char buffer[MAX_BUFFER_SIZE] = {0};
	if (recv(_fd, &buffer, MAX_BUFFER_SIZE, 0) < 0)
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
			if (i + (++len) == input.size() || input.at(i) == '\n')
				return ;
		first = input.substr(i, len);
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
	this->headers["Content-Length"] = std::to_string(contentLength);
	this->body = content;
}

void	Request::extractBody(std::string& input)
{
	size_t	i;
	size_t	len;

	this->body = "";
	i = input.find("\r\n\r\n");
	if (i == std::string::npos)
		return ;
	i += 4;
	if (!this->get("Transfer-Encoding").compare("chunked"))
		return (this->handleChunks(input, i));
	len = atoi(this->get("Content-Length").c_str());
	if (!len)
		len = atoi(this->get("content-length").c_str());
	for (size_t j = 0; j < len; j++)
		this->body.append(1, input[i++]);
}

void	Request::parse(char *buffer)
{
	std::string	input(buffer);

	this->extractMethod(input);
	this->extractTarget(input);
	this->extractVersion(input);
	this->extractHeaders(input);
	this->extractBody(input);
}

const std::string	Request::get(std::string toGet)
{
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
	if (!this->body.empty())
		std::cout << "Body: " << this->body << std::endl;
	std::cout << "---------------------" << std::endl;
	std::cout << std::endl;
}