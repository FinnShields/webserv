/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simpleServer.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fshields <fshields@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 12:16:17 by fshields          #+#    #+#             */
/*   Updated: 2024/06/12 13:00:39 by fshields         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "simpleServer.hpp"
#include "Request.hpp"

int	main()
{
	Request r1;
	int	listen_fd;
	int	conn_fd;
	int opt = 1;
	struct sockaddr_in serv_addr;
	char buffer[1024];
	std::string msg = "HTTP/1.1 200 OK\nContent-Type: text/plain\n\nHello, World!";

	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_fd == -1)
	{
		std::cerr << "socket error" << std::endl;
		return (1);
	}
	if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) 
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
    }
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(SERVER_PORT);
	
	if (bind(listen_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1)
	{
		std::cerr << "bind error" << std::endl;
		return (1);
	}
	if (listen(listen_fd, 10) == -1)
	{
		std::cerr << "listen error" << std::endl;
		return (1);
	}
	while (true)
	{
		std::cout << "Waiting for connection on port " << SERVER_PORT << std::endl;
		conn_fd = accept(listen_fd, (sockaddr *) NULL, NULL);
		if (conn_fd == -1)
			std::cerr << "accept error" << std::endl;
		else
			std::cout << "connection established" << std::endl;
		if (send(conn_fd, &msg, 5, 0) == -1)
			std::cerr << "send error" << std::endl;
		else
			std::cout << "sent message" << std::endl;
		if (recv(conn_fd, buffer, 1024, 0) == -1)
			std::cerr << "recv error" << std::endl;
		r1.parse(buffer);
		r1.display();
		close(conn_fd);
	}
}