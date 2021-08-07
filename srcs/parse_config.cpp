#include "webserv.hpp"

/********************************************************************/
/* ------------------- STATIC GLOBAL VARIABLES -------------------- */

static std::string const	directive[11] = {
	"listen",
	"server_name",
	"root",
	"index",
	"accepted_methods",
	"autoindex",
	"client_max_body_size",
	"error_page",
	"cgi_pass",
	"upload_path",
	"return"
};

static functionPtr const	f[11] = {
	&set_listen,
	&set_server_names,
	&set_root,
	&set_index,
	&set_accepted_methods,
	&set_autoindex,
	&set_client_max_body_size,
	&set_error_page,
	&set_cgi,
	&set_upload_path,
	&set_return
};

/********************************************************************/
/* ---------------------- PARSING FUNCTIONS ----------------------- */

/********************************************************************/
/* 																	*/
/********************************************************************/

void	parse_config(webserv& webserv, std::string path)
{
	std::ifstream in(path.c_str());
	std::stringstream ss;
	ss << in.rdbuf();
	if (errno)
	{
		if (in)
			in.close();
		throw std::runtime_error(strerror(errno));
	}
	for (size_t pos = 0; pos < ss.str().size(); pos++)
	{
		if ((pos = ss.str().find_first_not_of(" \n\t\v\f\r", pos)) == std::string::npos)
			break ;
		else if (ss.str()[pos] == '#')
		{
			if ((pos = ss.str().find("\n", pos)) == std::string::npos)
				break ;
		}
		else
		{
			size_t tmp = ss.str().find_first_of(" \n\t\v\f\r{", pos);
			if (!ss.str().substr(pos, tmp - pos).compare("server"))
			{
				int openBrace = 0;
				pos = ss.str().find_first_not_of(" \n\t\v\f\r", tmp);
				if (pos == std::string::npos || ss.str()[pos] != '{')
					throw std::runtime_error("server: expected a '{'");
				tmp = ss.str().find_first_of("{}", ++pos);
				if (ss.str()[tmp] == '{')
				{
					while (ss.str()[tmp] == '{')
					{
						openBrace++;
						tmp = ss.str().find_first_of("{}", ++tmp);
						while (openBrace && ss.str()[tmp] == '}')
						{
							openBrace--;
							tmp = ss.str().find_first_of("{}", ++tmp);
						}
					}
				}
				if (tmp == std::string::npos)
					throw std::runtime_error("unexpected end of file. Expected a '}'");
				parse_server(webserv, ss.str().substr(pos + 1, tmp - pos - 1));
				pos = tmp;
			}
			else
				throw std::runtime_error(SSTR("http: " << ss.str().substr(pos, tmp - pos) << ": no such context"));
		}
	}
	in.close();
}

/********************************************************************/
/* Syntax:	server { ... }											*/
/* Default:	-														*/
/* Context:	http													*/
/********************************************************************/

void	parse_server(webserv& webserv, std::string str)
{
	server server;
	size_t pos = -1;
	std::vector<std::string> locations;
	if (str.find_first_not_of(" \n\t\v\f\r") == std::string::npos)
		throw std::runtime_error("server: missing body");
	while (str[++pos])
	{
		pos = str.find_first_not_of(" \t\v\f\r", pos);
		if (str[pos] == '#')
			pos = str.find("\n", pos);
		if (str[pos] == '\n')
			continue ;
		else
		{
			size_t tmp = str.find_first_of(" \n\t\v\f\r{", pos);
			if (!str.substr(pos, tmp - pos).compare("location"))
			{
				pos = str.find_first_not_of(" \n\t\v\f\r", tmp);
				if (str[pos] == '{')
					throw std::runtime_error("location: missing prefix");
				tmp = str.find_first_of("{}", pos);
				if (tmp == std::string::npos || str[tmp] == '}')
					throw std::runtime_error("location: expected a '{'");
				else if (str[tmp + 1] == '}')
					throw std::runtime_error("location: missing body");
				tmp = str.find("}", pos);
				locations.push_back(str.substr(pos, tmp - pos - 1));
				pos = tmp;
			}
			else
			{
				std::string key = str.substr(pos, tmp - pos);
				std::vector<std::string> values = get_params(str.substr(tmp, str.find_first_of("#\n", tmp) - tmp));
				int i;
				for (i = 0; i < 12; i++)
				{
					if (!key.compare(directive[i]))
					{
						(*f[i])(webserv, &server, values);
						break ;
					}
				}
				if (i == 12)
					throw std::runtime_error(SSTR("server: " << key << ": no such directive"));
				pos = str.find("\n", tmp);
			}
		}
	}
	edit_server(webserv, server);
	parse_locations(webserv, server, locations);
	webserv.servers.push_back(server);
}

/********************************************************************/
/* Syntax:	location name { ... }									*/
/* Default:	-														*/
/* Context:	server													*/
/********************************************************************/

void	parse_locations(webserv& webserv, server& server, std::vector<std::string> locations)
{
	std::vector<std::string> names;
	for (std::vector<std::string>::iterator it = locations.begin(); it != locations.end(); it++)
	{
		location location(server);
		size_t pos = (*it).find_first_of(" \t\v\f\r{");
		location.name = (*it).substr(0, pos);
		for (size_t i = 0; i < server.locations.size(); i++)
			for (size_t j = 0; j < names.size(); j++)
				if (server.locations[i].name == location.name)
					throw std::runtime_error("location: duplicate prefix");
		names.push_back(location.name);
		pos = (*it).find_first_not_of(" \n\t\v\f\r", pos);
		if (pos == std::string::npos || (*it)[pos] != '{')
			throw std::runtime_error("location: expected a '{'");
		if ((*it).find_first_not_of(" \n\t\v\f\r", ++pos) == std::string::npos)
			throw std::runtime_error("location: missing body");
		while ((*it)[++pos])
		{
			pos = (*it).find_first_not_of(" \n\t\v\f\r", pos);
			if ((*it)[pos] == '#')
				pos = (*it).find("\n", pos);
			if ((*it)[pos] == '\n')
				continue ;
			else
			{
				size_t tmp = (*it).find_first_of(" \t\v\f\r", pos);
				std::string key((*it).substr(pos, tmp - pos));
				std::vector<std::string> values = get_params((*it).substr(tmp, (*it).find_first_of("#\n", tmp) - tmp));
				int i;
				for (i = 0; i < 12; i++)
				{
					if (!key.compare(directive[i]))
					{
						(*f[i])(webserv, &location, values);
						break ;
					}
				}
				if (i == 12)
					throw std::runtime_error(SSTR("location: " << key << ": no such directive"));
				pos = (*it).find("\n", pos);
			}
		}
		edit_location(server, location);
		server.locations.push_back(location);
	}
	for (size_t i = 0; i < server.locations.size(); i++)
	{
		if (server.locations[i].name == "/")
			break ;
		else if (i == server.locations.size() - 1)
		{
			std::cout << RED << "Adding default location '/'\n" << RESET; // TESTING
			location location(server);
			location.name = "/";
			edit_location(server, location);
			server.locations.push_back(location);
		}
	}
	names.clear();
}

/********************************************************************/
/* ----------------------- SETTER FUNCTIONS ----------------------- */

/********************************************************************/
/* Syntax:	listen [address:]port;									*/
/* Default:	listen 0.0.0.0:8000;									*/
/* Context:	server													*/
/********************************************************************/

void	set_listen(webserv& webserv, context* cxt, std::vector<std::string> values)
{
	(void)webserv;
	if (!dynamic_cast<struct server*>(cxt))
		throw std::runtime_error("listen: wrong context");
	server* server = dynamic_cast<struct server*>(cxt);
	if (!values.size())
		throw std::runtime_error("listen: missing parameter");
	if (values.size() > 1)
		throw std::runtime_error("listen: invalid syntax");
	size_t new_port = 8000;
	size_t pos = values[0].find(":");
	if (pos == std::string::npos)
	{
		pos = values[0].find_first_not_of("0123456789");
		if (pos == std::string::npos)
		{
			new_port = param_to_u_int(values[0]);
			for (size_t i = 0; i < server->port.size(); i++)
				if (new_port == server->port[i])
					throw std::runtime_error("listen: duplicate port");
			server->host.push_back("0.0.0.0");
			server->port.push_back(new_port);
		}
		else
			throw std::runtime_error("listen: missing or invalid port");
	}
	else
	{
		std::string tmp(values[0].substr(pos + 1));
		if (tmp.find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("listen: invalid port");
		new_port = param_to_u_int(tmp);
		for (size_t i = 0; i < server->port.size(); i++)
			if (new_port == server->port[i])
				throw std::runtime_error("listen: duplicate port");
		server->host.push_back(values[0].substr(0, pos));
		server->port.push_back(new_port);
	}
}

/********************************************************************/
/* Syntax:	server_name name ...;									*/
/* Default:	server_name "";											*/
/* Context:	server													*/
/********************************************************************/

void	set_server_names(webserv& webserv, context* cxt, std::vector<std::string> values)
{
	(void)webserv;
	if (!dynamic_cast<struct server*>(cxt))
		throw std::runtime_error("server_names: wrong context");
	server* server = dynamic_cast<struct server*>(cxt);
	if (!server->names.empty())
		throw std::runtime_error("server_names: duplicate directive");
	if (!values.size())
		throw std::runtime_error("server_names: missing parameter");
	for (size_t i = 0; i < values.size(); i++)
	{
		for (size_t j = 0; j < server->names.size(); j++)
			if (!values[i].compare(server->names[j]))
				throw std::runtime_error("server_names: duplicate values");
		server->names.push_back(values[i]);
	}
}

/********************************************************************/
/* Syntax:	root path;												*/
/* Default:	root www;												*/
/* Context:	server, location										*/
/********************************************************************/

void	set_root(webserv& webserv, context* cxt, std::vector<std::string> values)
{
	(void)webserv;
	if (!cxt->root.empty())
		throw std::runtime_error("root: duplicate directive");
	if (!values.size())
		throw std::runtime_error("root: missing parameter");
	if (values.size() > 1)
		throw std::runtime_error("root: invalid syntax");
	cxt->root = conform_path(values[0], false);
	if ((cxt->root[0] != '/' && !is_directory(webserv.cwd + std::string(cxt->root)))
		|| !is_directory(std::string(cxt->root)))
		throw std::runtime_error(SSTR("root: " << values[0] << ": no such directory"));
}

/********************************************************************/
/* Syntax:	index file ...;											*/
/* Default:	index index.html;										*/
/* Context:	server, location										*/
/********************************************************************/

void	set_index(webserv& webserv, context* cxt, std::vector<std::string> values)
{
	(void)webserv;
	if (!cxt->index.empty())
		throw std::runtime_error("index: duplicate directive");
	if (!values.size())
		throw std::runtime_error("index: missing parameter");
	for (size_t i = 0; i < values.size(); i++)
	{
		for (size_t j = 0; j < cxt->index.size(); j++)
			if (!values[i].compare(cxt->index[j]))
				throw std::runtime_error("index: duplicate values");
		cxt->index.push_back(values[i]);
	}
}

/********************************************************************/
/* Syntax:	accepted_methods method ...;							*/
/* Default:	accepted_methods method GET;							*/
/* Context:	server, location										*/
/********************************************************************/

void	set_accepted_methods(webserv& webserv, context* cxt, std::vector<std::string> values)
{
	(void)webserv;
	if (!values.size())
		throw std::runtime_error("accepted_methods: missing parameter");
	if (!cxt->accepted_methods.empty())
		throw std::runtime_error("accepted_methods: duplicate directive");
	for (size_t i = 0; i < values.size(); i++)
	{
		if (values[i] != "GET" && values[i] != "POST"
			&& values[i] != "DELETE" && values[i] != "NONE"
			&& values[i] != "HEAD" && values[i] != "PUT" // extra
			&& values[i] == "CONNECT" && values[i] == "OPTIONS" // not implemented
			&& values[i] == "TRACE" && values[i] == "PATCH") // not implemented
		{
			// if (values[i] == "HEAD" && values[i] == "PUT"
			// 	&& values[i] == "CONNECT" && values[i] == "OPTIONS"
			// 	&& values[i] == "TRACE" && values[i] == "PATCH")
			// 	throw std::runtime_error("accepted_methods: method not supported");
			throw std::runtime_error("accepted_methods: invalid value");
		}
		for (size_t j = 0; j < cxt->accepted_methods.size(); j++)
			if (values[i] == cxt->accepted_methods[j])
				throw std::runtime_error("accepted_methods: duplicate values");
		if (cxt->accepted_methods.size() && values[i] == "NONE")
			throw std::runtime_error(SSTR("accepted_methods: cannot assign 'NONE', '" << cxt->accepted_methods[0] << "' already present"));
		if (cxt->accepted_methods.size() && cxt->accepted_methods[0] == "NONE")
			throw std::runtime_error(SSTR("accepted_methods: cannot assign '" << values[i] << "', 'NONE' already present"));
		cxt->accepted_methods.push_back(values[i]);
	}
}

/********************************************************************/
/* Syntax:	autoindex on | off;										*/
/* Default:	autoindex off;											*/
/* Context:	server, location										*/
/********************************************************************/

void	set_autoindex(webserv& webserv, context* cxt, std::vector<std::string> values)
{
	(void)webserv;
	if (cxt->autoindex_set)
		throw std::runtime_error("autoindex: duplicate directive");
	if (!values.size())
		throw std::runtime_error("autoindex: missing parameter");
	if (values.size() > 1)
		throw std::runtime_error("autoindex: invalid syntax");
	if (values[0].compare("on") && values[0].compare("off"))
		throw std::runtime_error("autoindex: invalid parameter");
	if (!values[0].compare("on"))
		cxt->autoindex = true;
	else
		cxt->autoindex = false;
	cxt->autoindex_set = true;
}

/********************************************************************/
/* Syntax:	client_max_body_size size;								*/
/* Default:	client_max_body_size 1m;								*/
/* Context:	server, location										*/
/********************************************************************/

void	set_client_max_body_size(webserv& webserv, context* cxt, std::vector<std::string> values)
{
	(void)webserv;
	if (cxt->client_max_body_size != -1)
		throw std::runtime_error("client_max_body_size: duplicate directive");
	if (!values.size())
		throw std::runtime_error("client_max_body_size: missing parameter");
	if (values.size() > 1)
		throw std::runtime_error("client_max_body_size: invalid syntax");
	char last = values[0][values[0].length() - 1];
	std::string tmp("0123456789kmg");
	if (tmp.find(last) == std::string::npos)
		throw std::runtime_error("client_max_body_size: invalid value");
	long mul = (last == 'k') ? 1000 : (last == 'm') ? 1000000 : (last == 'g') ? 1000000000 : 1;
	if (last == 'k' || last == 'm' || last == 'g')
		tmp = values[0].substr(0, values[0].length() - 1);
	else
		tmp = values[0];
	if (tmp.find_first_not_of("0123456789") != std::string::npos)
		throw std::runtime_error("client_max_body_size: invalid value");
	cxt->client_max_body_size = mul * param_to_u_int(tmp);
}

/********************************************************************/
/* Syntax:	error_page code ... uri;								*/
/* Default:	error_page code www/errors/<code>.html					*/
/* Context:	server, location										*/
/********************************************************************/

void	set_error_page(webserv& webserv, context* cxt, std::vector<std::string> values)
{
	if (values.size() < 2)
		throw std::runtime_error("error_page: missing parameter");
	std::string path(values[values.size() - 1]);
	for (size_t i = 0; i < values.size() - 1; i++)
	{
		if (values[i].find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("error_page: invalid parameter");
		size_t code = param_to_u_int(values[i]);
		if (code < 100 || code > 599)
			throw std::runtime_error("error_page: invalid status code");
		std::map<size_t, std::string>::iterator it = cxt->error_pages.begin();
		for (; it != cxt->error_pages.end(); it++)
			if (code == it->first && !it->second.empty())
				throw std::runtime_error("error_page: duplicate assignment");
		cxt->error_pages[code] = conform_path(path, false);
		if ((cxt->error_pages[code][0] != '/' && !is_file(webserv.cwd + cxt->error_pages[code]))
			|| !is_file(cxt->error_pages[code]))
			throw std::runtime_error(SSTR("error_page: " << path << ": no such file"));
	}
}

/********************************************************************/
/* Syntax:	cgi_pass path;											*/
/* Default:	-														*/
/* Context:	location												*/
/********************************************************************/

void	set_cgi(webserv& webserv, context* cxt, std::vector<std::string> values)
{
	(void)webserv;
	if (!dynamic_cast<struct location*>(cxt))
		throw std::runtime_error("cgi_pass: wrong context");
	location* location = dynamic_cast<struct location*>(cxt);
	if (strncmp(location->name.c_str(), "*.", 2))
		throw std::runtime_error("cgi_pass: directive not allowed");
	if (location->cgi)
		throw std::runtime_error("cgi_pass: duplicate directive");
	if (!values.size())
		throw std::runtime_error("cgi_pass: missing parameter");
	if (values.size() > 1)
		throw std::runtime_error("cgi_pass: invalid syntax");
	location->cgi_ext = location->name.substr(1);
	location->cgi_pass = conform_path(values[0], false);
	if ((location->cgi_pass[0] != '/' && !is_file(webserv.cwd + std::string(location->cgi_pass)))
		|| !is_file(std::string(location->cgi_pass)))
		throw std::runtime_error(SSTR("cgi_pass: " << values[0] << ": no such file"));
	location->cgi = true;
}

/********************************************************************/
/* Syntax:	upload_path path;										*/
/* Default:	upload_path <server_root>/tmp/uploads/;					*/
/* Context:	server, location										*/
/********************************************************************/

void	set_upload_path(webserv& webserv, context* cxt, std::vector<std::string> values)
{
	(void)webserv;
	if (!values.size())
		throw std::runtime_error("upload_path: missing parameter");
	if (values.size() > 1)
		throw std::runtime_error("upload_path: invalid syntax");
	if (!cxt->upload_path.empty())
		throw std::runtime_error("upload_path: duplicate directive");
	cxt->upload_path = conform_path(values[0], true);
}

/********************************************************************/
/* Syntax:	return code [uri];										*/
/* 			return uri;												*/
/* Default:	-														*/
/* Context:	server, location										*/
/********************************************************************/

void	set_return(webserv& webserv, context* cxt, std::vector<std::string> values)
{
	(void)webserv;
	std::map<int, std::string> status_codes;
	if (dynamic_cast<struct server*>(cxt))
		status_codes = dynamic_cast<struct server*>(cxt)->status_codes;
	else if (dynamic_cast<location*>(cxt))
		status_codes = dynamic_cast<struct location*>(cxt)->server->status_codes;
	else
		throw std::runtime_error("return: parsing error");
	if (cxt->return_code != -1)
		throw std::runtime_error("return: duplicate directive");
	if (!values.size())
		throw std::runtime_error("return: missing parameter");
	if (values.size() > 2)
		throw std::runtime_error("return: invalid syntax");
	if (values[0].find_first_not_of("0123456789") != std::string::npos)
	{
		if (values.size() != 2)
		{
			cxt->return_code = 307;
			cxt->redirect = values[0];
		}
		else
			throw std::runtime_error(SSTR("return: " << values[0] << ": invalid parameter"));
	}
	else
	{
		int code = param_to_u_int(values[0]);
		if (status_codes[code].empty())
			throw std::runtime_error(SSTR("return: " << code << ": status code not supported"));
		if (values.size() != 2 && code / 100 == 3)
			throw std::runtime_error("return: missing parameter");
		if (values.size() == 2 && code / 100 != 3)
			throw std::runtime_error(SSTR("return: " << code << ": not a redirection code"));
		cxt->return_code = code;
		if (values.size() == 2)
		{
			std::string uri(values[1]);
			// if (uri.rfind(":") != std::string::npos && strncmp(uri.c_str(), "http://", 7))
			// 	throw std::runtime_error("return: uri: invalid syntax\nusage: [http://hostname:port]/path");
			cxt->redirect = uri;
		}
	}
}

/********************************************************************/
/* ---------------------- EDITING FUNCTIONS ----------------------- */

/********************************************************************/
/*			Any unset directives are set to their default.			*/
/********************************************************************/

void	edit_server(webserv& webserv, server& server)
{
	if (!server.host.size())
	{
		server.host.push_back("0.0.0.0");
		server.port.push_back(8000);
	}
	for (size_t i = 0; i < server.host.size(); i++)
		if (server.host[i] == "localhost")
			server.host[i] = "127.0.0.1";
	static std::vector<size_t> ports;
	for (size_t i = 0; i < server.port.size(); i++)
	{
		for (size_t j = 0; j < ports.size(); j++)
			if (server.port[i] == ports[j])
				goto present;
		server.default_server = true;
		ports.push_back(server.port[i]);
	}
	present:
	if (server.names.empty())
		server.names.push_back("");
	if (server.root.empty())
		server.root = "www";
	if (server.index.empty())
		server.index.push_back("index.html");
	if (server.accepted_methods.empty())
		server.accepted_methods.push_back("GET");
	if (server.client_max_body_size == -1)
		server.client_max_body_size = 1000000;
	std::map<size_t, std::string>::iterator it = server.error_pages.begin();
	for (; it != server.error_pages.end(); it++)
		if (it->second.empty())
			it->second = SSTR(webserv.cwd << ERROR_DIR << it->first << ".html");
	if (server.upload_path.empty())
		server.upload_path = server.root + "/tmp/uploads/";
}

/********************************************************************/
/*			Any unset directives inherit from the server.			*/
/********************************************************************/

void	edit_location(server& server, location& location)
{
	if (!strncmp(location.name.c_str(), "*.", 2) && !location.cgi)
		throw std::runtime_error(SSTR("location: " << location.name << ": no cgi_pass specified"));
	if (location.root.empty())
		location.root = server.root;
	if (location.index.empty())
		location.index = server.index;
	if (location.accepted_methods.empty())
		location.accepted_methods = server.accepted_methods;
	if (location.client_max_body_size == -1)
		location.client_max_body_size = server.client_max_body_size;
	std::map<size_t, std::string>::iterator it = location.error_pages.begin();
	for (; it != location.error_pages.end(); it++)
		if (it->second.empty())
			it->second = server.error_pages[it->first];
	if (location.error_pages.empty())
		location.error_pages = server.error_pages;
	if (location.upload_path.empty())
		location.upload_path = server.upload_path;
}
