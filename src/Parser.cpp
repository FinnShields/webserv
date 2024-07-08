#include "Parser.hpp"	

Parser::Parser(){}

Parser::~Parser(){}

Parser::Parser(const Parser& other){
	(void)other;
}

Parser& Parser::operator=(const Parser& other){
	(void)other;
	return *this;
}

Parser::Parser(const std::string filename): _filecontent(""), _token(tok::eof),	_endcontent(0),	_size(0), _position(0),	_tok_begin(0), _tok_end(0)
{
	_filecontent = readFile(filename);
	_size = _filecontent.size();
	parseFile();
}

std::string Parser::readFile(const std::string filename) const
{
	std::ifstream	infile(filename);
	if (!infile.is_open())
		throw  std::ios_base::failure("Parser::readFile can't open " + filename);
	std::cout << "Reading file" << std::endl;
	std::stringstream	buffer;
	buffer << infile.rdbuf();
	infile.close();
	return buffer.str();
}

Parser::tok Parser::peek(){
	while (_position < _size && isspace(_filecontent.at(_position)))
		++_position;
	if (_position >= _size)
		return tok::eof;
	switch (_filecontent.at(_position))
	{
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

Parser::tok Parser::getToken(){
	_token = peek();
	_tok_begin = _position;
	_tok_end = _position +
		(_token == tok::server ? 6 :
		_token == tok::group ? 5 :
		_token == tok::semicol ? 1 :
		_token == tok::open ? 1 :
		_token == tok::close ? 1 : 0);
	if (_token == tok::word)
		while (_tok_end < _size && !std::isspace(_filecontent.at(_tok_end)) && _filecontent.at(_tok_end) != ';')
     			++_tok_end;
	_position = _tok_end;
	return _token;
}

bool Parser::isAnyWord(tok token)
{
	return (token == tok::word ||
		token == tok::group ||
		token == tok::server);
}

std::string Parser::parseWord()
{
	if (isAnyWord(getToken()) == 0)
		throw std::runtime_error("Syntax error: expected word, but got: " + leftoverString());
	size_t len = _tok_end - _tok_begin;
	return _filecontent.substr(_tok_begin, len);
}

t_vector_str	Parser::parseWordList()
{
	t_vector_str value_list;
	while (isAnyWord(peek()))
		value_list.push_back(parseWord());
	if (getToken() != tok::semicol)
		throw std::runtime_error("Syntax error: expected semicolumn, but got: " + leftoverString());
	return value_list;
}

t_group Parser::parseGroupSetting()
{
	if (getToken() != tok::open)
		throw std::runtime_error(
			"Syntax error: expected {, but got: "
			+ leftoverString());
	t_group group;
	std::string keyword; // = parseWord();
	t_vector_str value_list;
	while (isAnyWord(peek()))
	{
		keyword = parseWord();
		value_list = parseWordList();
		group[keyword] = value_list;
	}
	if (getToken() != tok::close)
		throw std::runtime_error(
			"Syntax error: expected } or group, but got: "
			+ leftoverString());
	return group;
	}

t_server Parser::parseServer()
{
	if (getToken() != tok::server)
		throw std::runtime_error(
			"Syntax error: expected keyword server, but got: "
			+ leftoverString());
	if (getToken() != tok::open)
		throw std::runtime_error(
			"Syntax error: expected {, but got: "
			+ leftoverString());
	_token = peek();
	if (!(_token == tok::group || _token == tok::close))
		throw std::runtime_error(
			"Syntax error: expected group or }, but got: "
			+ leftoverString());
	t_server server;
	std::string dir;
	t_group	group;
	while (peek() == tok::group)
	{
		getToken();
		dir = parseWord();
		group = parseGroupSetting();
		server[dir] = group;
	}
	if (peek() != tok::close)
		throw std::runtime_error(
			"Syntax error: expected }, but got: "
			+ leftoverString());
	getToken();
	return server;
}

std::vector<t_server>& Parser::parseFile()
{
	while (peek() == tok::server)
		_data.push_back(parseServer());
	if (peek() != tok::eof)
		throw std::runtime_error(
			"Syntax error: expected server or EOF, but got: "
			+ leftoverString());
	return _data;
}

std::string Parser::leftoverString()
{
	//if (getToken() == tok::eof)
	if (peek() == tok::eof)
		return "EOF";
	std::string read_str = _filecontent.substr(0, _position);
    size_t new_line_count = std::count(read_str.begin(), read_str.end(), '\n') + 1;
	size_t new_line_prev = _filecontent.rfind('\n', _position);
	if (new_line_prev == std::string::npos)
		new_line_prev = 0;
	else
		new_line_prev++;
	size_t new_line_next = _filecontent.find('\n', _position);
	if (new_line_next == std::string::npos)
		new_line_next = _size;
	size_t pos = _position - new_line_prev;
	getToken();
	std::string line = _filecontent.substr(new_line_prev, new_line_next - new_line_prev);
	std::string line_err = line;
	for (size_t i = 0; i < line_err.size(); ++i)
	{
		if (!isspace(line_err[i]) &&
			i > _tok_begin - new_line_prev && 
			i < _tok_end - new_line_prev)
		{
			line_err[i] = '~';
		}
		else if (!isspace(line_err[i]))
			line_err[i] = ' ';
		if (pos < line_err.size())
			line_err[pos] = '^';
	}
	std::stringstream ss;
    ss << _filecontent.substr(_tok_begin, _tok_end - _tok_begin) 
		<< ", see line " << new_line_count << ":" << pos << "\n"
		<< line << "\n"
		<< line_err;
	return ss.str();
}


std::ostream& operator<<(std::ostream& os, const t_vector_str& vs)
{
    for (const std::string& s : vs)
        os << s << ", ";
    return os;
}

std::ostream& operator<<(std::ostream& os, const  t_group& l)
{
	for (auto& pair : l)
		std::cout << "\t\t" 
            << pair.first << "=(" 
            << pair.second << ");\n";
	return os;
}

std::ostream& operator<<(std::ostream& os, const  t_server& l)
{
	for (auto& [key, value] : l)
		std::cout << "\tsettings group:" << key << "\n"
            << value;
	return os;
}

std::ostream& operator<<(std::ostream& os, const  std::vector<t_server>& file)
{
    int i = -1;
	for (auto& server : file)
		std::cout << "server " << ++i << "\n"
            << server;
	return os;
}

size_t Parser::size()
{
	return _data.size();
}

std::vector<t_server>& Parser::get()
{
	return _data;
}

t_server Parser::get(size_t server)
{
	t_server ret;
	if (server < _data.size())
		ret = _data.at(server);
	return ret;
}

t_group Parser::get(size_t server, std::string group)
{
	return get(server)[group];
}

t_vector_str Parser::get(size_t server, std::string group, std::string key)
{
	return get(server, group)[key];
}

std::string Parser::get(size_t server, std::string group, std::string key, size_t num)
{
	t_vector_str vec = get(server, group, key);
	std::string str;
	if (num < vec.size())
		str = vec[num];
	return str;
}


bool Parser::isValidIP(const t_vector_str& vec) {
	if (vec.empty() || vec.size() > 1 || vec[0].empty())
		return false;
    std::stringstream ss(vec[0]);
    std::string token;
	int count = 0;
    while (std::getline(ss, token, '.')) {
		if (++count > 4)
			return false;
		if (token.empty() || token.size() > 3)
			return false;
		try {
			int num = std::stoi(token);
			if (num < 0 || num > 255)
				return false;
		}
		catch (const std::invalid_argument&){
            return false;
		}
		catch (const std::out_of_range&) {
            return false;
        }
    }
	if (count != 4)
    	return false;
	return true;
}

bool Parser::isValidPort(const t_vector_str& vec) {
	if (vec.empty() || vec.size() > 1 || vec[0].empty())
		return false;
	try {
		int num = std::stoi(vec[0]);
		if ( num < 1 || num > 65535)
				return false;
	}
	catch (const std::invalid_argument&){
        return false;
	}
	catch (const std::out_of_range&) {
        return false;
    }
	return true;
}

bool Parser::isValidNumber(const t_vector_str& vec, int limit_min, int limit_max) {
	if (vec.empty() || vec.size() > 1 || vec[0].empty())
		return false;
	try {
		int num = std::stoi(vec[0]);
		if ( num < limit_min || num > limit_max)
				return false;
	}
	catch (const std::invalid_argument&){
        return false;
	}
	catch (const std::out_of_range&) {
        return false;
    }
	return true;
}

bool Parser::isValidMethod(const t_vector_str& vec){
	t_vector_str mtd =  {"GET", "POST", "DELETE"};
	for (auto& m: vec){
		if (find(mtd.begin(), mtd.end(), m) == mtd.end())
			return false;
	}
	return true;
}

void Parser::isValid(){
	int srv_num = -1;
	if (_data.empty())
		throw std::runtime_error(", no server in config file.\n");
	for (auto& server : _data){
		std::cout << "Server " << ++srv_num << "  ";
		if (server.empty())
			throw std::runtime_error(", empty server.\n");
		if (server["main"].empty())
			throw std::runtime_error(", no main or it is empty\n");
		if (!isValidPort(server["main"]["listen"]))
			throw std::runtime_error(", none or invalid port\n");
		if (!isValidIP(server["main"]["host"]))
			throw std::runtime_error(", none or invalid IP\n");
		
		for (auto& [group_name, group_data]: server){
			if (group_data.empty())
				throw std::runtime_error(", empty group\n");
			if (group_name.empty())
				throw std::runtime_error(", empty group name");
			if (!(group_name == "main" || group_name[0] == '/'))
				throw std::runtime_error(", invalid group: " + group_name + "\n");
			t_vector_str values = group_data["client_max_body_size"];
			//std::cout << "->" << values <<  "<-n";
			if (!values.empty() && !isValidNumber(values, 10000, 30000000))
				throw std::runtime_error(", invalid client_max_body_size in group: " + group_name + "\n");
			values = group_data["limit_except"];
//			std::cout << values;
			if (!values.empty() && !isValidMethod(values))
				throw std::runtime_error(", invalid limit_except in group: " + group_name + "\n");
		}
		std::cout <<  " is OK.\n";
	}
}