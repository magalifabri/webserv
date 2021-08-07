#include "webserv.hpp"

std::string conform_path(std::string& param, bool trailing_slash)
{
	if (param.substr(0, 2) == "./")
		param.replace(0, 2, "");
	if (trailing_slash && param[param.length() - 1] != '/')
		param.append("/");
	if (!trailing_slash && param[param.length() - 1] == '/')
		param = param.substr(0, param.length() - 1);
	return param;
}

size_t	param_to_u_int(std::string value)
{
	size_t tmp;
	std::istringstream iss(value);
	if (!(iss >> tmp) || tmp < 0)
		throw std::runtime_error("param_to_u_int: parsing error");
	return tmp;
}

std::vector<std::string>	get_params(std::string params)
{
	std::vector<std::string> v;
	std::istringstream ss(params);
	while (ss)
	{
		std::string tmp;
		ss >> tmp;
		if (tmp == ";")
			return v;
		if (tmp[tmp.length() - 1] == ';')
		{
			v.push_back(tmp.substr(0, tmp.length() - 1));
			return v;
		}
		v.push_back(tmp);
	}
	throw std::runtime_error("unexpected end of parameter. Expected a ';'");
}
