#ifndef PARCER_HPP
# define PARCER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm> 

/* here we follow the EBNF given next:
	S -> server S'
	S' -> server S' | eps
	server -> 'server' '{' group_list '}'
	group_list -> group group_list | eps
	group -> 'group' identifier '{' key_value_list '}'
	key_value_list -> key_value key_value_list | eps
	key_value -> identifier value_list ';'
	value_list -> identifier value_list | eps
	identifier -> [non whitespace character]*  exclude ; server group
	eps = empty set
*/

using t_vector_str = std::vector<std::string>;
using t_group = std::map<std::string, t_vector_str>;
using t_server = std::map<std::string, t_group>;

class Parser
{
	public:
		Parser();
		~Parser();
		Parser(Parser &);
		Parser& operator=(Parser&);
		Parser(std::string);
		std::string readFile(std::string filename) const;
		enum class tok {server, group, word, semicol, //newline, 
			open, close, eof};
		void skipComment();
		tok peek();
		bool isAnyWord(tok token);
		tok getToken();
		std::vector<t_server>& parseFile();
		t_server parseServer();
		t_group parseGroupSetting();
		t_vector_str parseWordList();
		std::string parseWord();
		std::string leftoverString();
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

std::ostream& operator<<(std::ostream& os, const t_vector_str& vs);
std::ostream& operator<<(std::ostream& os, const t_group& l);
std::ostream& operator<<(std::ostream& os, const t_server& l);
std::ostream& operator<<(std::ostream& os, const std::vector<t_server>& file);


#endif


