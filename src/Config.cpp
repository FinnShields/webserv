#include "Config.hpp"	

/*
enum class Config::tok {
	server, group, word, 
	semicol, //newline,   comment,
	open, close, eof};
*/

Config::Config(){}

Config::~Config(){}

Config::Config(Config& other){
	(void)other;
}

Config& Config::operator=(Config& other){
	(void)other;
	return *this;
}

Config::Config(std::string filename):
	_filecontent(""),
	_token(tok::eof),
	_endcontent(0),
	_size(0),
	_position(0),
	_tok_begin(0),
	_tok_end(0)
{
	_filecontent = readFile(filename);
	_size = _filecontent.size();
	parseFile();
}

std::string Config::readFile(std::string filename) const{
	std::ifstream	infile(filename);
	if (!infile.is_open())
		throw  std::ios_base::failure("Config::readFile can't open " + filename);
	std::cout << "Reading file" << std::endl;
	std::stringstream	buffer;
	buffer << infile.rdbuf();
	infile.close();
	return buffer.str();
}

Config::tok Config::peek(){
	size_t i;
	for (i = _position; i < _size && isspace(_filecontent.at(i));){
		++i;
	}
	_position = i;
	if (_position >= _size)
		return tok::eof;
	switch (_filecontent.at(_position)){
		case '#':
			if (_filecontent.find('\n',_position) == std::string::npos)
				return tok::eof;
			_position = _filecontent.find('\n',_position) + 1;
			if (_position >= _size)
				return tok::eof;
			return peek();
		case '{':
			return tok::open;
		case '}':
			return tok::close;
		case ';':
			return tok::semicol;
		default:
			break ;
	}
	if (_filecontent.find("group",_position) == _position && 
			std::isspace(_filecontent.at(_position + 5)))
		return tok::group;
	else if (_filecontent.find("server",_position) == _position && 
			std::isspace(_filecontent.at(_position + 6)))
		return tok::server;
	return tok::word;
}

Config::tok Config::getToken(){
	_token = peek();
	_tok_begin = _position;
	_tok_end = _position;
	switch (_token){
		case tok::server:
			_position += 6;
			_tok_end = _position;
			break;
		case tok::group:
			_position += 5;
			_tok_end = _position;
			break;
		case tok::word:
			while (_tok_end < _size && 
					!std::isspace(_filecontent.at(_tok_end)) &&
					_filecontent.at(_tok_end) != ';'){
     			++_tok_end;
    		}
			_position = _tok_end;
			break;
		case tok::semicol:
			_position ++;
			_tok_end = _position;
			break;
		case tok::open:
			_position ++;
			_tok_end = _position;
			break;
		case tok::close:
			_position++;
			_tok_end = _position;
			break;
		case tok::eof:
			break;
		default:
			break;
	}
	return _token;
}

bool Config::isAnyWord(tok token){
	return (token == tok::word ||
		token == tok::group ||
		token == tok::server);
}

std::string Config::parseWord(){
	if (isAnyWord(peek()) == 0){
		throw std::runtime_error(
			"Syntax error: expected word, but got:\n"
			+ leftoverString());
	}
	getToken();
	size_t len = _tok_end - _tok_begin;
	return _filecontent.substr(_tok_begin, len);
}

t_vector_str	Config::parseWordList(){
	t_vector_str value_list;
	while (isAnyWord(peek())){
		value_list.push_back(parseWord());
	}
	if (peek() == tok::semicol)
		getToken();
	else
		throw std::runtime_error(
			"Syntax error: expected semicolumn, but got:\n"
			+ leftoverString());
	return value_list;
}

t_group Config::parseGroupSetting(){
	if (peek() != tok::open)
		throw std::runtime_error(
			"Syntax error: expected {, but got:\n"
			+ leftoverString());
	getToken();
	t_group group;
	std::string keyword; // = parseWord();
	t_vector_str value_list;
	while (isAnyWord(peek())){
		keyword = parseWord();
		value_list = parseWordList();
		group[keyword] = value_list;
	}
	if (peek() != tok::close)
		throw std::runtime_error(
			"Syntax error: expected } or group, but got:\n"
			+ leftoverString());
	getToken();
	return group;
}

t_server Config::parseServer(){
	if (peek() != tok::server)
		throw std::runtime_error(
			"Syntax error: expected keyword server, but got:\n"
			+ leftoverString());
	getToken();
	if (peek() != tok::open)
		throw std::runtime_error(
			"Syntax error: expected {, but got:\n"
			+ leftoverString());
	getToken();
	t_server server;
	std::string dir;
	t_group	group;
	std::cout << "parseServer\n";
	while (peek() == tok::group){
		getToken();
		dir = parseWord();
		group = parseGroupSetting();
		server[dir] = group;
	}
	if (peek() != tok::close)
		throw std::runtime_error(
			"Syntax error: expected }, but got:\n"
			+ leftoverString());
	getToken();
	return server;
}

std::vector<t_server>& Config::parseFile(){
	while (peek() == tok::server){
		_data.push_back(parseServer());
	}
	if (peek() != tok::eof)
		throw std::runtime_error(
			"Syntax error: expected server or EOF, but got:\n"
			+ leftoverString());
	return _data;
}

std::string Config::leftoverString(){
	if (getToken() == tok::eof)
		return "EOF";
	std::string read_str = _filecontent.substr(0, _position);
    size_t new_line_count = std::count(read_str.begin(), read_str.end(), '\n');
	std::cout << "in line number " << new_line_count << "   ";
	return _filecontent.substr(_tok_begin, _tok_end - _tok_begin);
}


std::ostream& operator<<(std::ostream& os, t_vector_str& vs){
    for (std::string& s : vs)
        os << s << ", ";
    return os;
}

std::ostream& operator<<(std::ostream& os, t_group& l){
	for (auto& pair : l)
		std::cout << "\t\t" 
            << pair.first << "=(" 
            << pair.second << ");\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, t_server& l){
	for (auto& [key, value] : l)
		std::cout << "\tsettings group:" << key << "\n"
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

size_t Config::size(){
	return _data.size();
}

std::vector<t_server>& Config::get(){
	return _data;
}

t_server Config::get(size_t server){
	t_server ret;
	if (server < _data.size())
		ret = _data.at(server);
	return ret;
}

t_group Config::get(size_t server, std::string group){
	return get(server)[group];
}

t_vector_str Config::get(size_t server, std::string group, std::string key){
	return get(server, group)[key];
}

std::string Config::get(size_t server, std::string group, std::string key, size_t num){
	t_vector_str vec = get(server, group, key);
	std::string str;
	if (num < vec.size())
		str = vec[num];
	return str;
}

