/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:41 by fshields          #+#    #+#             */
/*   Updated: 2024/09/30 10:07:39 by bsyvasal         ###   ########.fr       */
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
		bool	extractMethod(std::string& input, size_t *pos);
		bool	extractTarget(std::string& input, size_t *pos);
		bool	extractVersion(std::string& input, size_t *pos);
		void	extractHeaders(std::string& input);
		bool	headerInvalidChar(char c, int nameOrContent);
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