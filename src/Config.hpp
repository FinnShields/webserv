/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Config.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: apimikov <apimikov@student.hive.fi>        +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/09/26 14:04:39 by apimikov          #+#    #+#             */
/*   Updated: 2024/09/26 15:32:42 by apimikov         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIG_HPP
# define CONFIG_HPP

#include "Parser.hpp"

/* This class is defined to keep and manage data extracted 	by parsing congifig file.*/

class Config
{
	public:
		const size_t index;
		~Config();
		Config(const Config&);
		Config(std::vector<t_server>& data, size_t index);
		Config& operator=(const Config &);
		size_t	size() const;
		std::vector<t_server>& getAll() const;
		t_server			getAll(size_t server) const;
		t_group 			getAll(size_t server, std::string group) const;
		t_vector_str 		getAll(size_t server, std::string group, std::string key) const;
		std::string			getAll(size_t server, std::string group, std::string key, size_t num) const;
		t_server 			get() const;
		t_group 			get(std::string group) const;
		t_vector_str 		get(std::string group, std::string key) const;
		std::string 		get(std::string group, std::string key, size_t num) const;
		std::vector<int> 	getInt(std::string group, std::string key) const;
		int 				getInt(std::string group, std::string key, size_t num) const;
		std::string 		getFirst(std::string group, std::string key, std::string default_value) const;
		int			 		getFirst(std::string group, std::string key, int default_value) const;
		t_vector_str 		getList(std::string group, std::string key, std::string default_value) const;
		std::vector<int> 	getList(std::string group, std::string key, int default_value) const;
		std::string			selectLocation(std::string target) const;
		t_vector_str 		getValues(size_t virt_index, std::string target, std::string key,  t_vector_str default_values) const;
        t_vector_str        getBestValues(size_t virt_index, std::string target, std::string key, t_vector_str default_values) const;
		std::map<size_t, std::vector<size_t>> realToVirtualHosts() const;
	private:
		Config();
		std::vector<t_server>& _data;
		size_t scoreMatch(const std::string& target, const std::string& location) const;
};

#endif


