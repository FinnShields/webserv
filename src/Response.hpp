/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: bsyvasal <bsyvasal@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/14 13:06:10 by bsyvasal          #+#    #+#             */
/*   Updated: 2024/09/06 13:55:41 by bsyvasal         ###   ########.fr       */
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
#define DEFAULT_MAX_BODY_SIZE {"30000000"}



class Request;
class Server;
using t_vector_str = std::vector<std::string>;


class Response
{
	private:
        static const std::string ABSOLUTE_PATH;
        static const std::string setAbsolutePath();

		int	_fd;
		Request &_req;
		Server &_srv;
        int _file;
		int	_code;
		std::string _message;
        std::ofstream _filestream;
        std::ifstream _filestream_read;
        std::string _fileName;
		size_t _fileCurrentSize;
		size_t _bodyMaxSize;
        std::string _boundary;
		std::string _target;
		size_t _index_virt;
        std::string _response;

		const std::string redirect();
		const std::string runCGI();
        const std::string get();
		const std::string post();
		const std::string deleteResp();
		const std::string load_file(std::string filename);
		const std::string load_directory_listing(std::string directoryPath);
		const std::string getErrorPage(int code);
		const std::string appendfile();

        int saveFile();        
        int deleteFile(const std::string &);
		std::string createCookie();
		bool isMethodValid(std::string &method);

        //Helper functions
        bool check_body_size();
        std::string getPath();
        bool isHtml(const std::string fileName);
        bool load_directory_entries(const std::string directoryPath, t_vector_str &directories, t_vector_str &files);
        std::string getFileName(const std::string filepath);
        int setFileName(std::vector<char> &bodyRaw);
        void setDirectoryToFileName();
        void RenameIfFileExists();
        void checkOtherBoundary(std::vector<char> &bodyRaw, size_t &end, size_t offset);
        int checkBodySize(std::vector<char> &bodyRaw);
        std::string htmlEscape(const std::string& s);
		size_t  findString(std::vector<char> bodyRaw, std::string str, size_t offset);
        char decodeChar(const char *ch);

	public:
		// Response();
		Response(int fd, Request &req, Server &srv);
		Response(const Response &copy);
		Response &operator=(const Response &assign);
		~Response();
		
		const std::string run();
        const std::string getNextChunk();
        bool hasMoreChunks() const;
		void display() const;
        int getcode() const;
};
#endif
