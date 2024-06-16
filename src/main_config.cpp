
/*enum class Config::tok {
	server, location, word, 
	semicol, newline,  // comment,
	open, close, eof};
*/

#include "Config.hpp"

std::ostream& operator<<(std::ostream& os, t_vector_str& vs){
    for (std::string& s : vs)
        os << s << ", ";
    return os;
}

std::ostream& operator<<(std::ostream& os, t_location& l){
	for (auto& pair : l)
		std::cout << "\t\t" 
            << pair.first << "=(" 
            << pair.second << ");\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, t_server& l){
	for (auto& [key, value] : l)
		std::cout << "\tlocation dir=" << key << "\n"
            << value;
	return os;
}

std::ostream& operator<<(std::ostream& os, std::vector<t_server>& file){
    int i = -1;
	for (auto& server : file)
		std::cout << "server " << ++i << "\n"
            << server;
	return os;
}

int main() {

    Config config("test_config.txt");
    std::vector<t_server>& data = config.parseFile();
    std::cout << data;
    return 0;
}
