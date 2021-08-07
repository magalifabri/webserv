#include "webserv.hpp"

server&	select_server(webserv& webserv, std::string client_ip, std::string host, int sd)
{
	std::vector<int> servers;
	for (size_t i = 0; i < webserv.servers.size(); i++)
		for (size_t j = 0; j < webserv.servers[i].host.size(); j++)
			if ((client_ip == webserv.servers[i].host[j]
				|| webserv.servers[i].host[j] == "0.0.0.0")
				&& webserv.port_map[sd] == webserv.servers[i].port[j])
				servers.push_back(i);
	std::vector<int>::iterator it;
	host = host.substr(0, host.find(":"));
	for (it = servers.begin(); it != servers.end(); it++)
		for (size_t i = 0; i < webserv.servers[*it].names.size(); i++)
			if (host == webserv.servers[*it].names[i])
				return webserv.servers[*it];
	for (it = servers.begin(); it != servers.end(); it++)
		if (webserv.servers[*it].default_server)
			return webserv.servers[*it];
	throw std::string("500 Internal Server Error");
}

static std::string	get_root(server server, std::string URI)
{
	while (!URI.empty())
	{
		for (size_t i = 0; i < server.locations.size(); i++)
			if (server.locations[i].name == URI)
				return server.locations[i].root;
		URI = URI.substr(0, URI.rfind('/'));
		for (size_t i = 0; i < server.locations.size(); i++)
			if (server.locations[i].name == URI)
				return server.locations[i].root;
		URI = URI.substr(0, URI.rfind('/') + 1);
	}
	return server.root;
}

location&	select_location(server server, std::string URI, std::string method)
{
	// std::cout << URI << '\n'; // TESTING
	if (URI[URI.length() - 1] != '/')
		URI += "/";
	while (!URI.empty())
	{
		for (size_t i = 0; i < server.locations.size(); i++)
			if (server.locations[i].name == URI)
				return server.locations[i];
		URI = URI.substr(0, URI.rfind('/'));
		for (size_t i = 0; i < server.locations.size(); i++)
			if (server.locations[i].name == URI)
				return server.locations[i];
		if (URI.rfind(".") != std::string::npos)
		{
			std::string ext = URI.substr(URI.rfind("."));
			for (size_t i = 0; i < server.locations.size(); i++)
			{
				if (method == "GET" && ext == ".bla")
					break ;
				if (server.locations[i].name == "*" + ext)
				{
					server.locations[i].root = get_root(server, URI);
					return server.locations[i];
				}
			}
		}
		URI = URI.substr(0, URI.rfind('/') + 1);
	}
	throw std::string("500 Internal Server Error");
}

bool	select_index(request& request, std::string* URI)
{
	for (size_t i = 0; i < request.location.index.size(); i++)
	{
		std::string tmp;
		if ((*URI)[URI->length() - 1] != '/')
			tmp = *URI + "/" + request.location.index[i];
		else
			tmp = *URI + request.location.index[i];
		struct stat buffer;
		if (!stat((request.location.root + tmp).c_str(), &buffer))
		{
			*URI = tmp;
			if (request.headers_map["target"] != "/")
				request.headers_map["target"] += "/" + request.location.index[i];
			else
				request.headers_map["target"] += request.location.index[i];
			// std::cout << "new target: " << request.headers_map["target"] << '\n'; // TESTING
			request.location = select_location(request.server,
				request.headers_map["target"], request.headers_map["method"]);
			request.location.server = &request.server;
			return true;
		}
	}
	return false;
}
