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

size_t Config::scoreMatch(const std::string& target, const std::string& location) const {
	size_t len = location.length();
	if (target.length() < len)
		return 0;
	if (target.substr(0, len).compare(location) == 0)
		return len;
	return 0;
}

std::string Config::selectLocation(std::string target) const {
	std::map<std::string, size_t> scores;
	std::string best_location;
	size_t best_score = 0;
	for (const auto& [location, value]: get()){
		size_t score = scoreMatch(target, location);
		scores[location] = score;
		if (score > best_score){
			best_score = score;
			best_location = location;
		}
	}
	if (best_location.empty())
		best_location = "main";
	return best_location;
}

t_vector_str Config::getValues(std::string target, std::string key, t_vector_str default_values) const
{
	std::string group = selectLocation(target);
//	std::cout << "group=" << group << "\n";
	t_vector_str vec = get()[group][key];
	if (0 < vec.size())
		return vec;
	return default_values;
}

t_vector_str Config::getValues(size_t virt_index, std::string target, std::string key, t_vector_str default_values) const
{
	std::string group = selectLocation(target);
//	std::cout << "group=" << group << "\n";
	t_vector_str vec = getAll(virt_index)[group][key];
	if (0 < vec.size())
		return vec;
	return default_values;
}

//void Config::setVirtualHosts() const
std::map<size_t, std::vector<size_t>> Config::realToVirtualHosts() const
{
	std::vector<size_t> indices;
	std::map<size_t, std::vector<size_t>> real_to_virt;
	for (size_t i = 0; i < _data.size(); i++)
	{
		auto it = std::find(indices.begin(), indices.end(), i);
		if (it != indices.end())
			continue;
		std::vector<size_t> indices_host;
		std::string port = getAll(i, "main", "listen", 0);
		std::string host = getAll(i, "main", "host", 0);
		std::string name = getAll(i, "main", "server_name", 0);
		for (size_t j = i + 1; j < _data.size(); j++)
		{
			if (port != getAll(j, "main", "listen", 0))
				continue;
			if (host != getAll(j, "main", "host", 0))
				continue;
			indices.push_back(j);
			indices_host.push_back(j);
		}
		real_to_virt[i] = indices_host;
	}
	return real_to_virt;
}


bool Config::isValid(){
	//validate server_name?
	//validate ip
	//validate port
	return false;
}