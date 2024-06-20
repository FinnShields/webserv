#include "Config.hpp"

Config::Config(){}

Config::~Config(){}

Config::Config(std::vector<t_server> data): _data(data) {}

Config::Config(Config& other){
	(void)other;
}

Config& Config::operator=(const Config other){
	this->_data = other._data;
	return *this;
}

size_t Config::size(){
	return _data.size();
}

std::vector<t_server> Config::get(){
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