#ifndef PARCECONF_HPP
# define PARCECONF_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

//# define TOKENS "{};\n#"

typedef std::vector<std::string> t_vector_str;
typedef std::map<std::string, t_vector_str> t_location;
typedef std::map<std::string, t_location> t_server;

/*
std::vector<Server> servers
server = std::map<std::string location, Config> locations
config = std::map<std::string key, values> setting
values = std::vector<std::string>
*/


class Config
{
	public:
		Config();
		~Config();
		Config(Config &);
		Config& operator=(Config&);
		Config(std::string);
		std::string readFile(std::string filename) const;
		//enum class tok;	
		enum class tok {server, location, word, semicol, newline, 
			open, close, eof};
		tok getToken();
		tok peek();
		void skipComment();
	//	void parseFile();
	//	void parseServer();
	//	void parseLocationList();
		t_location parseLocation();
		t_vector_str	parseWordList();
		std::string parseWord();
		tok		_token;
	private:
		std::string _filecontent;
		size_t	_endcontent;
		size_t	_size;	
		size_t	_position;
		size_t	_tok_begin;	
		size_t	_tok_end;
		std::vector<t_server> _data;
};

#endif

/* EBNF

S -> server S'
S' -> server S' | eps
server -> 'server' '{' main location_list '}'
main -> 'main' '{' key_value_list '}'
location_list -> location location_list | eps
location -> 'location' identifier '{' key_value_list '}'
key_value_list -> key_value key_value_list | eps
key_value -> identifier value_list ';'
value_list -> identifier value_list | eps
identifier -> [a-zA-Z_][a-zA-Z0-9_]*
eps = empty set


*/
