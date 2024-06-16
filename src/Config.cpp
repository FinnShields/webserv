#include "Config.hpp"	

/*
enum class Config::tok {
	server, location, word, 
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
	_endcontent(0),
	_size(0),
	_position(0),
	_tok_begin(0),
	_tok_end(0)
{
	_filecontent = readFile(filename);
	/*
	std::cout <<  "File content:\n-----------------------\n" 
		<< _filecontent 
		<< "\n----------- end of file -------- \n"
		<< std::endl;
	*/
	_size = _filecontent.size();
	//std::cout <<  "File size =" << _size << "\n"; 
}

std::string Config::readFile(std::string filename) const{
	std::ifstream	infile(filename);
	if (!infile)
	{
		std::cout << "Unable to open the file " << filename << std::endl;
		return ("");
	}
	std::stringstream	buffer;
	buffer << infile.rdbuf();
	return buffer.str();
}


/*  void Config::skipComment(){
	_position = _filecontent.find('\n',_position) + 1;
}  */

Config::tok Config::peek(){
	//std::string tokens = TOKENS;
	// skip spaces
	size_t i;
	for (i = _position; i < _size && isspace(_filecontent.at(i));){
		++i;
	}
	_position = i;
	if (_position >= _size)
		return tok::eof;
	//std::cout << _position << " c=" <<_filecontent.at(_position) << std::endl;

	switch (_filecontent.at(_position)){
		case '#':
			//std::cout << "peek: comment\n";
			//skipComment();
			if (_filecontent.find('\n',_position) == std::string::npos)
				return tok::eof;
			_position = _filecontent.find('\n',_position) + 1;
			if (_position >= _size)
				return tok::eof;
			return peek();
		case '{':
//			std::cout << "peek: open {\n";
			return tok::open;
		case '}':
//			std::cout << "peek: close }\n";
			return tok::close;
		case ';':
//			std::cout << "peek: semicol ;\n";
			return tok::semicol;
		default:
			break ;
	}
	if (_filecontent.find("location",_position) == _position)
	{
//		std::cout << "peek: location\n";
		return tok::location;
	}
	else if (_filecontent.find("server",_position) == _position)
	{
//		std::cout << "peek: location\n";
		return tok::server;
	}
//	std::cout << "peek: word\n";
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
		//	std::cout << "add new server\n";
			break;
		case tok::location:
			_position += 8;
			_tok_end = _position;
		//	std::cout << "add new location to current server\n";
			break;
		case tok::word:
			while (_tok_end < _size && 
				!std::isspace(_filecontent.at(_tok_end)) &&
				_filecontent.at(_tok_end) != ';'){
     			++_tok_end;
    		}
			_position = _tok_end;
		//	std::cout << "add new word to current server and location\n";
			break;
		case tok::semicol:
			_position ++;
			_tok_end = _position;
		//	std::cout << "semicolumn\n";
			break;
		case tok::open:
			//_open_par++;
			_position ++;
			_tok_end = _position;
		//	std::cout << "{ open \n";
			break;
		case tok::close:
			//_open_par--;
			_position++;
			_tok_end = _position;
		//	std::cout << "close } \n";
			break;
		case tok::eof:
			//std::cout << "EOF\n";
			break;
		default:
			break;
	}
	return _token;
}

std::string Config::parseWord(){
	if (peek() != tok::word)
		 throw std::runtime_error(
			"Syntax error: expected word, but got:\n"
			+ leftoverString());
	getToken();
	size_t len = _tok_end - _tok_begin;
	return _filecontent.substr(_tok_begin, len);
}

t_vector_str	Config::parseWordList(){
	t_vector_str value_list;
	while (peek() == tok::word){
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

t_location Config::parseLocationDict(){
	if (peek() != tok::open)
		throw std::runtime_error(
			"Syntax error: expected {, but got:\n"
			+ leftoverString());
	getToken();
	t_location location;
	std::string keyword; // = parseWord();
	t_vector_str value_list;
	while (peek() == tok::word){
		keyword = parseWord();
		value_list = parseWordList();
		location[keyword] = value_list;
	}
	if (peek() != tok::close)
		throw std::runtime_error(
			"Syntax error: expected }, but got:\n"
			+ leftoverString());
	getToken();
	return location;
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
	t_location	location;
	while (peek() == tok::location){
		getToken();
		dir = parseWord();
		location = parseLocationDict();
		server[dir] = location;
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
	//tok token = ;
	if (getToken() == tok::eof)
		return "EOF";
	return _filecontent.substr(_tok_begin, _tok_end - _tok_begin);
}


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
