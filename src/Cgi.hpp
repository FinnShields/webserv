/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cgi.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/26 14:04:53 by apimikov          #+#    #+#             */
/*   Updated: 2024/09/26 15:23:11 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <cstring>
#include <ctime>
#include <algorithm>
#include <sys/wait.h>
#include <fcntl.h>


#include "Server.hpp"
#include "Request.hpp"

class Server;
class Request;

class Cgi
{
    private:
        Request& _request;
        const Server& _server;
        std::string _target;
        const size_t _index_virt;
        std::string _path;
        size_t _pos_cgi;
        size_t _pos_dot;
        size_t _pos_query;
        size_t _pos_info;
        std::string _ext;
        std::string _target_file_path;
        std::string _target_path_info;
        std::string _target_query;
        std::string _target_foldername;
        std::string _target_file_name;
        size_t _cgi_type;
        std::string _cgi_path;

        
        std::map<std::string,std::string> _env_map;
        char** _argv;
        char** _envp;
        std::string _response;
        int _fd_from_cgi[2];
        int _fd_to_cgi[2];
        int _pid;
        int _status;

        Cgi();
        void setExtension();
        void analizeTarget();
        bool isImplemented();
        void setEnvMap();
        void setEnv();
        void cleanEnv();
        std::string readFromFd(int);
        int _access();
        void _runChildCgi();
        bool _wait();
    public:
        Cgi(Request&, const Server&, const size_t, std::string path);
        Cgi(const Cgi&);
        Cgi &operator=(const Cgi&);
        ~Cgi();
        
        void start();
        void runCmd();
        std::string getResponse();
        int getStatus();
		ssize_t writeToPipe(const void *buf, size_t count);
		std::string readFromPipe();
		int get_pipereadfd();
		int get_writefd();
		void closeWritePipe();
        
};

#endif
