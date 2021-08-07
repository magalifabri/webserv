#include "webserv.hpp"

bool URI_is_accessible(const std::string &path)
{
	std::ifstream file(path);

	/* If ifstream wasn't successfully created, the file isn't accessable */
	if (!file)
		return (false);
	else
	{
		file.close();
		return (true);
	}
}



bool check_URI_existence(const request &request, std::string &target)
{
	if (is_file(request.location.root + target))
		std::cout << "FILE" << '\n'; // TESTING
	else if (is_directory(request.location.root + target))
	{
		std::cout << "DIR" << '\n'; // TESTING
		if (request.location.autoindex == true)
			return (true);
		else
			if (select_index(request.location, &target) == false)
				throw (404);
	}
	else
		throw (404);

	return (false);
}



void handle_location_aliasing(const request &request, std::string &target)
{
	// if (std::strncmp(request.location.name.c_str(), "*.", 2)
	// 	&&
	if (!is_directory(request.conf.root + request.location.name)
		&& !is_file(request.conf.root + request.location.name))
	{
		if (request.location.name[request.location.name.length() - 1] == '/')
			target = target.replace(0, request.location.name.length() - 1, "");
		else
			target = target.replace(0, request.location.name.length(), "");
	}
	
	if (target.empty())
		target += "/";
}



std::string create_response_page(const request &request, const std::string &target)
{
	std::string response_body = get_response_body(request.location.root + target);
	std::string content_type = get_content_type("200 OK", target);
	std::string response_header = SSTR(
		"HTTP/1.1 200 OK\n"
		<< "Content-Type: " << content_type << "\n"
		<< "Content-Length: " << response_body.length() << "\n"
		<< "\n");

	TESTING_print_response(response_header, response_body);

	return (response_header + response_body);
}



/*
- check if cgi
- check if the target (requested URI) is an alias (a location that refers to
  another location on the server) and if so change the target to the URI it
  refers to
- check if the target exists or if the target should trigger the return of an
  autoindex page
- check if the target is accessible
- create and return a response that holds the resource the URI indicated
*/

std::string	handle_GET_or_HEAD_request(webserv& webserv, request &request)
{
	if (request.location.cgi)
		return (execute_cgi(webserv, request, request.location));

	/* --- HANDLE LOCATION ALIASING --- */

	std::string target = request.headers_map["target"];
	// std::cout << "target: '" << target << "'" << std::endl; // TESTING

	handle_location_aliasing(request, target);
	std::cout << "target after aliasing: '" << target << "'" << std::endl; // TESTING
	bool autoindex = check_URI_existence(request, target);
	if (autoindex == true)
		return (create_autoindex_response(request.location, target));
	// std::cout << "target: '" << target << "'" << std::endl; // TESTING
	if (URI_is_accessible(request.location.root + target) == false)
		throw (403);
	std::string response = create_response_page(request, target);
	return (response);
}
