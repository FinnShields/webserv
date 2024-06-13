#include <		

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

void	ParceConf::readFile(){
	std::ifstream	infile(_filename);
	if (!infile)
	{
		std::cout << "Unable to open the file " << _filename << std::endl;
		return ("");
	}
	std::stringstream	buffer;
	buffer << infile.rdbuf();
	buffer.str();
}
