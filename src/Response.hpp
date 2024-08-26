/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:06:10 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/08/23 14:54:42 by bsyvasal         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <dirent.h>
#include <iostream>
#include <algorithm>
#include <string>
#include <cstring>
#include <filesystem>
#include "Request.hpp"
#include "Server.hpp"

#define STATUS_LINE_200 "HTTP/1.1 200 OK\r\n"
#define STATUS_LINE_201 "HTTP/1.1 201 Created\r\n"
#define STATUS_LINE_204 "HTTP/1.1 204 No Content\r\n"
#define RESPONSE_404 "HTTP/1.1 404 Page not found\r\nContent-Type: text/plain\r\n"
#define RESPONSE_405 "HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain\r\n"
#define RESPONSE_500 "HTTP/1.1 500 Internal Server Error\r\nContent-Type: text/plain\r\n"
#define RESPONSE_501 "HTTP/1.1 501 Not Implemented\r\nContent-Type: text/plain\r\n\r\nError: Method not recognized or not implemented"
#define DEFAULT_INDEX {"index.html"}
#define DEFAULT_MAX_BODY_SIZE {"1000"}



class Request;
class Server;
using t_vector_str = std::vector<std::string>;

class Response
{
	private:
		int	_fd;
		Request &_req;
		Server &_srv;
        int _file;
        std::ofstream _filestream;
        std::ifstream _filestream_read;
        std::string _fileName;
        std::string _boundary;
		std::string _target;
		size_t _index_virt;
        std::string _response;

		std::string get();
		std::string post();
		std::string deleteResp();
		std::string load_file(std::string filename);
		std::string load_directory_listing(std::string directoryPath);
		const std::string appendfile();
        int saveFile();
        
        int deleteFile(const std::string &);
		std::string createCookie();
		bool isMethodValid(std::string &method);
		std::string runCGI();

        
        //Helper functions
        bool check_body_size();
        std::string getPath();
        bool isHtml(const std::string fileName);
        bool load_directory_entries(const std::string directoryPath, t_vector_str &directories, t_vector_str &files);
        std::string getFileName(const std::string filepath);
        std::string htmlEscape(const std::string& s);
		void replacePercent20withSpace(std::string &str);
		std::string getErrorPage(int code);
        

		Response();
		Response(const Response &copy);
		Response &operator=(const Response &assign);
	public:
		Response(int fd, Request &req, Server &srv);
		~Response();
		
		const std::string run();
        const std::string getNextChunk();
        bool hasMoreChunks();
};
#endif
