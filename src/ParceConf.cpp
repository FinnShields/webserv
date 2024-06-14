#include "ParceConf.hpp"	

enum class ParceConf::tok {
	server, location, word, 
	semicol, newline,  // comment,
	open, close, eof};

ParceConf::ParceConf(){}

ParceConf::~ParceConf(){}

ParceConf::ParceConf(ParceConf& other){
	(void)other;
}

ParceConf& ParceConf::operator=(ParceConf& other){
	(void)other;
	return *this;
}

ParceConf::ParceConf(std::string filename):
	_filecontent(""),
	_endcontent(0),
	_size(0),
	_position(0),
	_tok_begin(0),
	_tok_end(0)
{
	_filecontent = readFile(filename);
	std::cout <<  "File content:\n-----------------------\n" 
		<< _filecontent 
		<< "\n----------- end of file -------- \n"
		<< std::endl;
	_size = _filecontent.size();
	std::cout <<  "File size =" << _size << "\n"; 
}

std::string ParceConf::readFile(std::string filename) const{
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


/*void ParceConf::skipComment(){
	_position = _filecontent.find('\n',_position) + 1;
}*/

ParceConf::tok ParceConf::peek(){
	std::string tokens = TOKENS;
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

ParceConf::tok ParceConf::getToken(){
	tok token;
	token = peek();
	_tok_begin = _position;
	_tok_end = _tok_begin;
	switch (token){
		case tok::server:
			_position += 6;
			std::cout << "add new server\n";
			break;
		case tok::location:
			_position += 8;
			std::cout << "add new location to current server\n";
			break;
		case tok::word:
			while (_tok_end < _size && !std::isspace(_filecontent.at(_tok_end))){
     			++_tok_end;
    		}
			_position = _tok_end;
			std::cout << "add new word to current server and location\n";
			break;
		case tok::semicol:
			_position ++;
			std::cout << "semicolumn\n";
			break;
		case tok::open:
			_position ++;
			std::cout << "{ open \n";
			break;
		case tok::close:
			_position ++;
			std::cout << "close } \n";
			break;
		case tok::eof:
			std::cout << "EOF\n";
			break;
		default:
			break;
	}
	return token;
}