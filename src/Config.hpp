#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm> 

#include "Parser.hpp"

/* This class is defined to keep and manage data extracted 
	by parsing congifig file. 
*/

class Config
{
	public:
		Config();
		~Config();
		Config(Config &);
		Config& operator=(Config&);
		Config(std::string);
		size_t	size();
		std::vector<t_server>& get();
		t_server get(size_t server);
		t_group get(size_t server, std::string group);
		t_vector_str get(size_t server, std::string group, std::string key);
		std::string get(size_t server, std::string group, std::string key, size_t num);
		
		/*class SyntaxError: public std::exception {
			public:
				const char* what() const noexcept override;
		};*/
	private:
		const std::vector<t_server> _data;
};

#endif


