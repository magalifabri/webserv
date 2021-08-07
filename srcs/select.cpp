#include "webserv.hpp"

/* --- SELECT SERVER --- */
server&	select_server(int sd, std::vector<server> servers, std::map<size_t, size_t> port_map, std::string host)
{
	host = host.substr(0, host.find(':'));
	// std::cout << GREEN << "Port: " << port_map[sd] << "\nHostname: " << host << RESET << '\n'; // TESTING

	// for (size_t i = 0; i < servers.size(); i++)
	size_t i; // TESTING
	for (i = 0; i < servers.size(); i++) // TESTING
		for (size_t j = 0; j < servers[i].port.size(); j++)
			if (port_map[sd] == servers[i].port[j])
				for (size_t k = 0; k < servers[i].names.size(); k++)
					if (host == servers[i].names[k])
						goto stop; // TESTING
						// return servers[i];
	// for (size_t i = 0; i < servers.size(); i++)
	for (i = 0; i < servers.size(); i++) // TESTING
		for (size_t j = 0; j < servers[i].port.size(); j++)
			if (port_map[sd] == servers[i].port[j])
				goto stop; // TESTING
				// return servers[i];
	stop: // TESTING
	// std::cout << GREEN << "Server #" << i << RESET << '\n'; // TESTING
	return servers[i]; // TESTING
}

std::string	get_root(server conf, std::string URI)
{
	while (!URI.empty())
	{
		for (size_t i = 0; i < conf.locations.size(); i++)
			if (conf.locations[i].name == URI)
				return conf.locations[i].root;
		URI = URI.substr(0, URI.rfind('/'));
		for (size_t i = 0; i < conf.locations.size(); i++)
			if (conf.locations[i].name == URI)
				return conf.locations[i].root;
		URI = URI.substr(0, URI.rfind('/') + 1);
	}
	return conf.root;
}

location&	select_location(server conf, std::string URI, std::string method)
{
	// std::cout << URI << '\n'; // TESTING
	if (URI[URI.length() - 1] != '/')
		URI += "/";
	while (!URI.empty())
	{
		for (size_t i = 0; i < conf.locations.size(); i++)
			if (conf.locations[i].name == URI)
				return conf.locations[i];
		URI = URI.substr(0, URI.rfind('/'));
		for (size_t i = 0; i < conf.locations.size(); i++)
			if (conf.locations[i].name == URI)
				return conf.locations[i];
		if (URI.rfind(".") != std::string::npos)
		{
			std::string ext = URI.substr(URI.rfind("."));
			for (size_t i = 0; i < conf.locations.size(); i++)
			{
				if (method == "GET" && ext == ".bla")
					break ;
				if (conf.locations[i].name == "*" + ext)
				{
					conf.locations[i].root = get_root(conf, URI);
					return conf.locations[i];
				}
			}
		}
		URI = URI.substr(0, URI.rfind('/') + 1);
	}
	throw std::runtime_error("500 Internal Server Error");
}

bool	select_index(const location& location, std::string* URI)
{
	for (size_t i = 0; i < location.index.size(); i++)
	{
		std::string tmp;
		if ((*URI)[URI->length() - 1] != '/')
			tmp = *URI + "/" + location.index[i];
		else
			tmp = *URI + location.index[i];
		struct stat buffer;
		if (!stat((location.root + tmp).c_str(), &buffer))
		{
			*URI = tmp;
			return true;
		}
	}
	return false;
}
