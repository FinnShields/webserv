/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fshields <fshields@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/06/12 08:43:41 by fshields          #+#    #+#             */
/*   Updated: 2024/06/12 13:56:14 by fshields         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <string>
# include <iostream>
# include <map>

class Request
{
	private:
		std::map<std::string, std::string> data;
	public:
		Request();
		Request(const Request&);
		Request& operator=(const Request&);
		~Request();
		void parse(char *buffer);
		std::string extractMethod(std::string& input);
		std::string extractTarget(std::string& input);
		std::string extractVersion(std::string& input);
		void display();
};

#endif