#ifndef PARCER_HPP
# define PARCER_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <regex>

#define CLIENT_MAX_BODY_SIZE 30000000

using t_vector_str = std::vector<std::string>;
using t_group = std::map<std::string, t_vector_str>;
using t_server = std::map<std::string, t_group>;

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

class Parser
{
	public:
		enum class tok {server, group, word, semicol, open, close, eof};

		Parser();
		~Parser();
		Parser(const Parser &);
		Parser& operator=(const Parser&);
		Parser(const std::string);

		std::string 			readFile(const std::string filename) const;
		void					skipComment();
		tok 					peek();
		bool					isAnyWord(tok token);
		tok						getToken();
		std::vector<t_server>&	parseFile();
		t_server 				parseServer();
		t_group 				parseGroupSetting();
		t_vector_str 			parseWordList();
		std::string 			parseWord();
		std::string 			leftoverString();
		size_t					size();

		std::vector<t_server>&	get();
		t_server				get(size_t server);
		t_group					get(size_t server, std::string group);
		t_vector_str			get(size_t server, std::string group, std::string key);
		std::string				get(size_t server, std::string group, std::string key, size_t num);

		bool isValidIP(const t_vector_str& vec);
		bool isValidPort(const t_vector_str& vec);
		bool isValidNumber(const t_vector_str& vec, int limit_max);
		bool isValidMethod(t_group& group_data);
		bool isValidSrvNameLabel(const std::string&);
		bool isValidSrvNameDNS(std::string&);
		bool isValidSrvName(t_group& group_data);
		void isValid();

	private:
		std::string _filecontent;
		tok		_token;
		size_t	_endcontent;
		size_t	_size;
		size_t	_position;
		size_t	_tok_begin;	
		size_t	_tok_end;
		std::vector<t_server> _data;
};

std::ostream& operator<<(std::ostream& os, const t_vector_str& vs);
std::ostream& operator<<(std::ostream& os, const t_group& l);
std::ostream& operator<<(std::ostream& os, const t_server& l);
std::ostream& operator<<(std::ostream& os, const std::vector<t_server>& file);


#endif


