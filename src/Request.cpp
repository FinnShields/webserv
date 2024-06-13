/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fshields <fshields@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:48 by fshields          #+#    #+#             */
/*   Updated: 2024/06/13 11:44:05 by fshields         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Request.hpp"

Request::Request()
{}

Request::~Request()
{}

Request& Request::operator=(const Request& r)
{
	this->data = r.data;
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

void	Request::parse(char *buffer)
{
	std::string	input(buffer);
	std::string	first;
	std::string second;
	size_t	len;

	this->data["method"] = this->extractMethod(input);
	this->data["request-target"] = this->extractTarget(input);
	this->data["http-version"] = this->extractVersion(input);
	for (size_t i = input.find('\n') + 1; i < input.size(); i++)
	{
		len = 0;
		while (input.at(i + len) != ':')
		{
			len ++;
			if (i + len == input.size())
				return ;
		}
		first = input.substr(i, len);
		i += len + 2;
		len = 0;
		while (input.at(i + len) != '\r' && i + len < input.size())
			len ++;
		second = input.substr(i, len);
		this->data[first] = second;
		i += len + 1;
	}
}

const std::string	Request::getMethod()
{
	if (this->data.empty())
		return ("");
	return (this->data["method"]);
}

const std::string	Request::getDir()
{
	if (this->data.empty())
		return ("");
	return (this->data["request-target"]);
}

void	Request::display()
{
	std::cout << std::endl;
	std::cout << "---------------------" << std::endl;
	std::map<std::string, std::string>::iterator it;
	std::cout << "method: " << this->data["method"] << std::endl;
	std::cout << "request-target: " << this->data["request-target"] << std::endl;
	for(it = this->data.begin(); it != this->data.end(); it++)
		if (!it->first.compare("method") && !it->first.compare("request-target"))
			std::cout << it->first << ": " << it->second << std::endl;
	std::cout << "---------------------" << std::endl;
	std::cout << std::endl;
}