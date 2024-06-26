#include "Config.hpp"

//Config::Config(): index(0){}

Config::~Config(){}

Config::Config(std::vector<t_server>& data, size_t i): index(i), _data(data) 
{
	if (i >= _data.size())
			throw  std::runtime_error("Config constructor: index is too big\n");
}

/*
Config::Config(Parser& pars, size_t index):
	_data(pars.getAll()),
	index(index) {}
*/

Config::Config(const Config& other): index(other.index), _data(other._data) {}

Config& Config::operator=(const Config &other)
{
	_data = other._data;
	return *this;
}

size_t Config::size() const
{
	return _data.size();
}

std::vector<t_server>& Config::getAll() const
{
	return _data;
}

t_server Config::getAll(size_t server) const
{
	t_server ret;
	if (server < _data.size())
		ret = _data.at(server);
	return ret;
}

t_group Config::getAll(size_t server, std::string group) const
{
	return getAll(server)[group];
}

t_vector_str Config::getAll(size_t server, std::string group, std::string key) const
{
	return getAll(server, group)[key];
}

std::string Config::getAll(size_t server, std::string group, std::string key, size_t num) const
{
	t_vector_str vec = getAll(server, group, key);
	std::string str;
	if (num < vec.size())
		str = vec[num];
	return str;
}


t_server Config::get() const
{
	return _data.at(index);
}

t_group Config::get(std::string group) const
{
	return get()[group];
}

t_vector_str Config::get(std::string group, std::string key) const
{
	return get()[group][key];
}

std::string Config::get(std::string group, std::string key, size_t num) const
{
	t_vector_str vec = get()[group][key];
	std::string str;
	if (num < vec.size())
		str = vec[num];
	return str;
}

std::vector<int> Config::getInt(std::string group, std::string key) const
{
	t_vector_str vec = get(group)[key];
	std::vector<int> ret;
	for (auto& n : vec){
		ret.push_back(std::stoi(n));
	}
	return ret;
}

int Config::getInt(std::string group, std::string key, size_t num) const
{
	std::vector<int> vec = getInt(group, key);
	int ret = 0;
	if (num < vec.size())
		ret = vec[num];
	return ret;
}

std::string Config::getFirst(std::string group, std::string key, std::string default_value) const
{
	t_vector_str vec = get()[group][key];
	if (0 >= vec.size())
		return default_value;
	return vec[0];
}

int Config::getFirst(std::string group, std::string key, int default_value) const
{
	std::vector<int> vec = getInt(group, key);
	if (0 < vec.size())
		return vec[0];
	return default_value;
}

t_vector_str Config::getList(std::string group, std::string key, std::string default_value) const
{
	t_vector_str vec = get()[group][key];
	if (0 < vec.size())
		return vec;
	vec.push_back(default_value);
	return vec;
}

std::vector<int> Config::getList(std::string group, std::string key, int default_value) const
{
	std::vector<int> ret = getInt(group, key);
	if (0 == ret.size()){
		ret.push_back(default_value);
	}
	return ret;
}
