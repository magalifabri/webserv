#include "webserv.hpp" 

std::string get_response(webserv &webserv, request &request)
{
	/* If a POST request is sent (via the upload.html page) or a DELETE request
	is sent (via the delete.html page), then the content of the request is
	processed separately. But apart from that the request is handled in the same
	manner as a regular GET request. */
	if (request.headers_map["method"] == "POST"
	&& request.headers_map["full-target"] == "/upload.html")
		return (handle_webform_POST(request.location, request));
	/* If a GET request is sent via the get_form page with a query string, the
	query string is parsed and a page is returned that lists the parsed data. */
	else if (request.headers_map["method"] == "GET"
	&& request.headers_map["full-target"].find("/get_form.html?") != std::string::npos)
		return (create_get_form_response(request));
	else if (request.headers_map["method"] == "DELETE")
		return (handle_DELETE_request(request.location, request));
	else if (request.headers_map["method"] == "POST")
		return (handle_POST_request(webserv, request.location, request));
	else if (request.headers_map["method"] == "PUT")
		return (handle_PUT_request(request.location, request));
	else
		return (handle_GET_or_HEAD_request(webserv, request));
		// return (handle_GET_or_HEAD_request(request));
}



bool request_method_allowed(request &request, const location &location)
{
	if (std::find(
		location.accepted_methods.begin(),
		location.accepted_methods.end(),
		request.headers_map["method"]) != location.accepted_methods.end())
		return (true);
	else
		return (false);
}



std::string create_redirection_response(request &request)
{
	std::string redirect = SSTR(
		"HTTP/1.1 " << request.conf.status_codes[request.location.return_code] << "\n" <<
		"Location: " << request.location.redirect << "\r\n\r\n");
	
	TESTING_print_response(redirect, "");
	
	return (redirect);
}



/*
- check if URI should trigger redirection
- check if method is allowed
- get and return an appropriate response
*/
std::string process_request(webserv& webserv, request &request)
{
	if (!request.location.redirect.empty())
		return (create_redirection_response(request));
	if (request_method_allowed(request, request.location) == false)
		throw (405);
	return (get_response(webserv, request));
}
