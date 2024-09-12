/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:41 by fshields          #+#    #+#             */
/*   Updated: 2024/09/10 14:32:42 by bsyvasal         ###   ########.fr       */
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
		std::string _method;
		std::string _target;
		std::string _version;
		std::map<std::string, std::string> _headers;
		std::vector<char> _bodyRawBytes;
		std::vector<char> _reqRaw;
		ssize_t _recvReturnTotal;
        ssize_t _bodyTotalSize;
        int _status;
		bool _chunkedReqComplete;
		bool _incompleteChunk;
		void parse();
		bool extractMethod(std::string& input);
		void extractTarget(std::string& input);
		void extractVersion(std::string& input);
		void extractHeaders(std::string& input);
		void extractBody();
		void handleChunks(char *reqArray, size_t i);
		void moreChunks();
        void resetBody();
        int readContentLength(int);
        int isCGI();
        bool    isWholeHeader();
	public:
		Request();
		Request(const Request&);
		Request& operator=(const Request&);
		~Request();
		int read(int _fd);
		const std::string get(std::string toGet);
		const std::string getHeader(std::string toGet);
		std::string& getRef(std::string toGet);
		std::vector<char> getBodyRawBytes();
        ssize_t getBodyTotalSize();
		void display();
		int getStatus();
};

#endif