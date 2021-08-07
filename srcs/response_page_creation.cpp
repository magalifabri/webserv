#include "webserv.hpp"

std::string create_accepted_methods_string(
	const std::vector<std::string> &accepted_methods)
{
	std::string allowed;

	allowed = "Allow: ";
	std::vector<std::string>::const_iterator it;
	for (it = accepted_methods.begin(); it != accepted_methods.end(); it++)
	{
		if (it != accepted_methods.begin())
			allowed += ", ";
		allowed += *it;
	}
	allowed += "\n";

	return (allowed);
}



std::string create_error_response(const int &error_code, location &location)
{
	std::string status = location.server->status_codes.at(error_code);
	std::string response_body = get_response_body(location.error_pages[error_code]);

	std::string allowed;
	if (status == "405 Method Not Allowed")
		allowed = create_accepted_methods_string(location.accepted_methods);

	std::string response_header = SSTR(
		"HTTP/1.1 " << status << "\n" <<
		"Content-Type: text/html\n" <<
		"Content-Length: " << response_body.length() << "\n" <<
		allowed <<
		"\n"
	);
	
	TESTING_print_response(response_header, response_body);

	return (response_header + response_body);
}



std::string create_2xx_response_page(
	const std::string &status, const std::string &location_header)
{
	std::string response_body;
	if (status == "200 OK")
		response_body = get_response_body("www/200.html");
	else
		response_body = get_response_body("www/201_created.html");

	std::string response_header = SSTR(
		"HTTP/1.1 " << status << "\n"
		<< "Content-Type: text/html\n"
		<< "Content-Length: " << response_body.length() << "\n"
		<< location_header
		<< "\n");

	TESTING_print_response(response_header, response_body);

	return (response_header + response_body);
}
