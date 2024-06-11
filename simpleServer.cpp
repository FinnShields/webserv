/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   simpleServer.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fshields <fshields@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/11 12:16:17 by fshields          #+#    #+#             */
/*   Updated: 2024/06/11 14:16:07 by fshields         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "simpleServer.hpp"

int	main()
{
	int	listen_fd;
	int	conn_fd;
	struct sockaddr_in serv_addr;
	char buffer[1024];
	std::string msg = "hello";

	listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (listen_fd == -1)
	{
		std::cerr << "socket error" << std::endl;
		return (1);
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
		if (recv(conn_fd, buffer, 1024, MSG_WAITALL) == -1)
			std::cerr << "recv error" << std::endl;
		else
			std::cout << "send message" << std::endl;
		std::cout << buffer << std::endl;
		close(conn_fd);
	}
}