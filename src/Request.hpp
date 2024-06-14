/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fshields <fshields@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:41 by fshields          #+#    #+#             */
/*   Updated: 2024/06/13 12:44:29 by fshields         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <iostream>
# include <map>
# include "Client.hpp"

class Request
{
	private:
		std::string method;
		std::string target;
		std::string version;
		std::map<std::string, std::string> headers;
		std::string body;
		void parse(char *buffer);
		std::string extractMethod(std::string& input);
		std::string extractTarget(std::string& input);
		std::string extractVersion(std::string& input);
	public:
		Request();
		Request(const Request&);
		Request& operator=(const Request&);
		~Request();
		void read(int _fd);
		const std::string get(std::string toGet);
		void display();
};

#endif