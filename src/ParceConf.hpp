#ifndef PARCECCONF_HPP
# define PARCECCONF_HPP

/*
std::vector<Server> servers
server = std::map<std::string location, Config> locations
config = std::map<std::string key, values> setting
values = std::vector<std::string>
*/

class ParceConf
{
	public:
		ParceConf();
		~ParceConf();
		ParceConf(ParceConf &);
		ParceConf& operator=(ParceConf&);
		ParceConf(std::string);
		void
	private:
		std::string _filename;
		std::string _content;
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
