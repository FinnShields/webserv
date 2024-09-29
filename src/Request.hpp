/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:41 by fshields          #+#    #+#             */
/*   Updated: 2024/09/29 18:07:06 by bsyvasal         ###   ########.fr       */
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
# include "Server.hpp"

class Cgi;
class Server;

class Request
{
	private:
		Server		*_srv;
		std::string _method;
		std::string _target;
		std::string	_version;
		std::map<std::string, std::string>	_headers;
		std::vector<char>	_bodyRawBytes;
		std::vector<char>	_reqRaw;
		std::vector<char>	_currentChunkBytes;
		ssize_t	_recvReturnTotal;
        ssize_t	_bodyTotalSize;
        int		_status;
		int		_currentChunkSize;
		bool	_chunkedReqComplete;
		bool	_cgi_flag;
		bool	_badrequest;
		bool	_headerComplete;
		void	parse();
		bool	extractMethod(std::string& input);
		void	extractTarget(std::string& input);
		void	extractVersion(std::string& input);
		void	extractHeaders(std::string& input);
		void	extractBody();
		int		chunkExtractNumber(char *reqArray, size_t &i, size_t max_size);
		void	handleChunks(char *reqArray, size_t i, size_t max_size);
		void 	chunkExtractBody(char *reqArray, size_t i, size_t max_size);
        void 	resetBody();
        int 	setCGIflag();
        bool	isWholeHeader();
		Request(const Request&);
		Request& operator=(const Request&);
	public:
		Request(Server *srv);
		~Request();
		
		int		read(int _fd);
		const std::string get(std::string toGet);
		const std::string getHeader(std::string toGet);
		std::map<std::string, std::string> getHeaders();
		std::string& getRef(std::string toGet);
		std::vector<char> &getBodyRawBytes();
        ssize_t	getBodyTotalSize();
		void	display();
		bool	isCGIflag();
		bool	IsBodyIncomplete();
		int		getStatus();
		bool 	isBadRequest();
};

#endif