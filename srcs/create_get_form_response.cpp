#include "webserv.hpp"

std::string create_form_data_html(const std::map<std::string, std::string> &URI_map)
{
	std::string html;
	for (std::map<std::string, std::string>::const_iterator it = URI_map.begin(); it != URI_map.end(); it++)
	{
		std::string key(it->first.begin(), it->first.end());
		std::string value(it->second.begin(), it->second.end());
		html += "<li>" + key + ": " + value + "</li>\n";
	}
	// std::cout << html << std::endl; // TESTING

	return (html);
}



std::string assemble_response(const std::vector<char> &file_vector)
{
	std::string page_html(file_vector.begin(), file_vector.end());
	if (page_html.empty() == true)
		throw (500);

	std::string response_header = SSTR(
		"HTTP/1.1 200 OK\n"
		<< "Content-Type: text/html\n"
		<< "Content-Length: " << page_html.length() << "\n"
		<< "\n"
	);

	return (response_header + page_html);
}



/*
- create an html list of the data received from the GET web form
- load a premade html template and insert the list into it (as well as
  an appropriate header)
- return a response with the customized template
*/
std::string create_get_form_response(request &request)
{
	request.parse_URI_query();
	/* create html list of form data to insert into template */
	std::string html = create_form_data_html(request.URI_query_map);
	/* convert index.html file - ifstream */
	std::vector<char> file_vector = create_html_template_vector(
		"www/templates/get_form_response_template.html");
	/* in the vector locate where to insert the html */
	insert_string_into_template(
		file_vector, html, "<!-- insert form data here -->");
	/* convert vector<char> - string */
	std::string response = assemble_response(file_vector);
	// TESTING_print_response(response_header, page_html);
	return (response);
}
