#ifndef PARCECCONF_HPP
# define PARCECCONF_HPP

class ParceConf
{
	public:
		ParceConf();
		~ParceConf();
		ParceConf(ParceConf &);
		ParceConf& operator=(ParceConf&);
	private:
		std::string str;
};

#endif
