#ifndef PARCECONF_HPP
# define PARCECONF_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

/* here we follow the EBNF given next:
	S -> server S'
	S' -> server S' | eps
	server -> 'server' '{' location_list '}'
	location_list -> location location_list | eps
	location -> 'location' identifier '{' key_value_list '}'
	key_value_list -> key_value key_value_list | eps
	key_value -> identifier value_list ';'
	value_list -> identifier value_list | eps
	identifier -> [non whitespace character]*  exclude ; server location
	eps = empty set
*/

using t_vector_str = std::vector<std::string>;
using t_location = std::map<std::string, t_vector_str>;
using t_server = std::map<std::string, t_location>;

class Config
{
	public:
		Config();
		~Config();
		Config(Config &);
		Config& operator=(Config&);
		Config(std::string);
		std::string readFile(std::string filename) const;
		enum class tok {server, location, word, semicol, //newline, 
			open, close, eof};
		void skipComment();
		tok peek();
		bool isAnyWord(tok token);
		tok getToken();
		std::vector<t_server>& parseFile();
		t_server parseServer();
		t_location parseLocationDict();
		t_vector_str parseWordList();
		std::string parseWord();
		std::string leftoverString();
		size_t	size();
		std::vector<t_server>& get();
		t_server get(size_t server);
		t_location get(size_t server, std::string group);
		t_vector_str get(size_t server, std::string group, std::string key);
		std::string get(size_t server, std::string group, std::string key, size_t num);
		
		/*class SyntaxError: public std::exception {
			public:
				const char* what() const noexcept override;
		};*/
	private:
		std::string _filecontent;
		tok		_token;
		size_t	_endcontent;
		size_t	_size;
		size_t	_position;
		size_t	_tok_begin;	
		size_t	_tok_end;
		//int		_open_par;
		std::vector<t_server> _data;
};

std::ostream& operator<<(std::ostream& os, t_vector_str& vs);
std::ostream& operator<<(std::ostream& os, t_location& l);
std::ostream& operator<<(std::ostream& os, t_server& l);
std::ostream& operator<<(std::ostream& os, std::vector<t_server>& file);


#endif


