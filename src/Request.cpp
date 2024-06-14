/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fshields <fshields@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:48 by fshields          #+#    #+#             */
/*   Updated: 2024/06/13 12:44:24 by fshields         ###   ########.fr       */
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

std::string Request::extractMethod(std::string& input)
{
	return (input.substr(0, input.find_first_of(' ')));
}

std::string Request::extractTarget(std::string& input)
{
	size_t start;
	size_t end;

	start = input.find_first_of(' ') + 1;
	end = 0;
	while (input.at(start + end) != ' ')
		end++;
	return (input.substr(start, end));
}

std::string Request::extractVersion(std::string& input)
{
	size_t start = input.find("HTTP");
	size_t end = 0;
	while (input.at(start + end) != '\r')
		end++;
	return (input.substr(start, end));
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

void	Request::parse(char *buffer)
{
	std::string	input(buffer);
	std::string	first;
	std::string second;
	size_t	len;

	this->method = this->extractMethod(input);
	this->target = this->extractTarget(input);
	this->version = this->extractVersion(input);

//add parsing for potential payload

	for (size_t i = input.find('\n') + 1; i < input.size(); i++)
	{
		len = 0;
		while (input.at(i + len) != ':')
			if (i + (++len) == input.size())
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

const std::string	Request::get(std::string toGet)
{
	if (!toGet.compare("method"))
		return (this->method);
	if (!toGet.compare("target"))
		return (this->target);
	if (!toGet.compare("version"))
		return (this->version);
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
	std::cout << "---------------------" << std::endl;
	std::cout << std::endl;
}