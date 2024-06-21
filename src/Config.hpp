#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "Parser.hpp"

/* This class is defined to keep and manage data extracted 
	by parsing congifig file.
*/

class Config
{
	public:
		~Config();
		Config(const Config&);
		Config(std::vector<t_server>& data, size_t index);
		Config& operator=(const Config);
		size_t	size() const;
		
		std::vector<t_server>& getAll() const;
		t_server getAll(size_t server) const;
		t_group getAll(size_t server, std::string group) const;
		t_vector_str getAll(size_t server, std::string group,
			std::string key) const;
		std::string getAll(size_t server, std::string group, std::string key, 
			size_t num) const;
		
		t_server get() const;
		t_group get(std::string group) const;
		t_vector_str get(std::string group, std::string key) const;
		std::string get(std::string group, std::string key, size_t num) const;

		std::vector<int> getInt(std::string group, std::string key) const;
		int getInt(std::string group, std::string key, size_t num) const;

		const size_t index;
	private:
		Config();
		std::vector<t_server>& _data;
};

#endif


