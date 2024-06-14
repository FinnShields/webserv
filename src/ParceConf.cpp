#include <		

enum class ParceConf::tok {server, location, word, semicol, newline, comment};

ParceConf::ParceConf(){}

ParceConf::ParceConf(std::string filename):_filename(filename){}

ParceConf::~ParceConf(){}

ParceConf::ParceConf(ParceConf& other){
	(void)other;
}

ParceConf& operator=(ParceConf& other){
	(void)other;
	return *this;
}

ParceConf::ParceConf(std::string filename){
	_filecontent = readFile(filename);
	_size = _filecontent.size();
}

std::string ParceConf::readFile(filename) const{
	std::ifstream	infile(filename);
	if (!infile)
	{
		std::cout << "Unable to open the file " << _filename << std::endl;
		return ("");
	}
	std::stringstream	buffer;
	buffer << infile.rdbuf();
	return buffer.str();
}
