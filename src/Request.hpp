/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:41 by fshields          #+#    #+#             */
/*   Updated: 2024/07/11 06:04:41 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <iostream>
# include <map>
# include <vector>
# include "Client.hpp"
# include "Cgi.hpp"

class Cgi;

class Request
{
	private:
		std::string method;
		std::string target;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;
		std::vector<char> bodyRawBytes;
		std::vector<char> reqRaw;
		ssize_t recvReturnTotal;
		void parse(std::vector<char> reqRaw);
		bool extractMethod(std::string& input);
		void extractTarget(std::string& input);
		void extractVersion(std::string& input);
		void extractHeaders(std::string& input);
		void extractBody(std::vector<char> reqRaw);
		void handleChunks(std::string& input, size_t i);
	public:
		Request();
		Request(const Request&);
		Request& operator=(const Request&);
		~Request();
		void read(int _fd);
		const std::string get(std::string toGet);
		const std::string getHeader(std::string toGet);
		std::string& getRef(std::string toGet);
		std::vector<char> getBodyRawBytes();
		void display();
};

#endif