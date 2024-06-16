#ifndef PARCECONF_HPP
# define PARCECONF_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

//# define TOKENS "{};\n#"

//typedef std::vector<std::string> t_vector_str;
//typedef std::map<std::string, t_vector_str> t_location;
//typedef std::map<std::string, t_location> t_server;


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
		enum class tok {server, location, word, semicol, newline, 
			open, close, eof};
		void skipComment();
		tok peek();
		tok getToken();
		std::vector<t_server>& parseFile();
		t_server parseServer();
		t_location parseLocationDict();
		t_vector_str parseWordList();
		std::string parseWord();
		tok		_token;
	private:
		std::string _filecontent;
		size_t	_endcontent;
		size_t	_size;
		size_t	_position;
		size_t	_tok_begin;	
		size_t	_tok_end;
		//int		_open_par;
		std::vector<t_server> _data;
};

//std::ostream&	operator<<(std::ostream& os, const t_vector_str& l);
//std::ostream&	operator<<(std::ostream& os, const t_location& l);

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
