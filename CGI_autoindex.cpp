// #include "webserv.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

# define SSTR(x) static_cast<std::ostringstream&>((std::ostringstream() << std::dec << x)).str() // convert ... to string

/*
std::string location.root
std::string location.name
std::string request.headers_map["full-target"]
std::string folder

target: /uploads/
location.root: www
location.name: /uploads/
folder: /uploads/

./CGI_autoindex.cgi /uploads/ www /uploads/ /uploads/
*/

std::string create_autoindex_response(
    std::string target,
    std::string location_root,
    std::string location_name,
    std::string folder)
{
	/* --- MAKE DIRECTORY LISTING --- */

	/* run ls with requested directory and store output in string (shell command - string) */
	// std::cout << "ls -p " + location_root + folder << '\n'; // TESTING
	std::string query = "ls -p " + location_root + folder;
	// std::cout << "query: " << query << std::endl; // TESTING
	char buf[128];
	std::string dir_list = "";
	FILE *pipe = popen(&query[0], "r");

	if (location_name[location_name.length() - 1] != '/')
		folder = location_name + folder;

	/* <li><a href=" | buf - \n | "> | buf - \n | </a></li> */
	while (fgets(buf, sizeof buf, pipe) != NULL)
	{
		dir_list += "<li><a href=\"";
		dir_list += folder + buf;
		// dir_list += buf;
		dir_list.pop_back(); // remove LF to put closing tag on same line
		dir_list += "\">";
		dir_list += buf;
		dir_list.pop_back();
		dir_list += "</a></li>\n";
	}
	// std::cout << "dir_list:" << std::endl << dir_list; // TESTING

	pclose(pipe);

	/* --- INSERT HEADER & DIR LIST INTO INDEX TEMPLATE --- */

	/* index.html file - ifstream */
	std::ifstream file_stream("www/templates/index_template.html");
	/* ifstream - vector<char> */
	std::vector<char> file_vector;
	std::copy(std::istream_iterator<char>(file_stream >> std::noskipws),
		std::istream_iterator<char>(), std::back_inserter(file_vector));

	/* locate in the vector where we want to insert the header */
	query = "<!-- insert header here -->";
	/* find string - vector<char> */
	std::vector<char>::iterator insert_pos = std::search(
		file_vector.begin(), file_vector.end(),
		&query[0], &query[query.length()]);
	/* remove string - vector<char> */
	file_vector.erase(insert_pos, insert_pos + query.length());
	/* insert string - vector<char> */
	file_vector.insert(insert_pos, folder.begin(), folder.end());

	/* locate in the vector where we want to insert the directory listing */
	query = "<!-- insert directory listing here -->";
	/* find string - vector<char> */
	insert_pos = std::search(
		file_vector.begin(), file_vector.end(),
		&query[0], &query[query.length()]);
	/* remove string - vector<char> */
	file_vector.erase(insert_pos, insert_pos + query.length());
	/* insert string - vector<char> */
	file_vector.insert(insert_pos, dir_list.begin(), dir_list.end());
	
	// TESTING_print_vector_char(file_vector);
	
	/* vector<char> - string */
	std::string index_html(file_vector.begin(), file_vector.end());
	// std::cout << "index_html: " << index_html << std::cout; // TESTING

	/* --- CREATE RESPONSE HEADER --- */

	std::string status = "200 OK";
	std::string location_header;

	/* If folder is unequal the original target this means a redirection occurred. */
	if (folder != target)
	{
		status = "308 Permanent Redirect";
		location_header = "Location: " + folder + "\n";
	}

	std::string response_header = SSTR(
		"HTTP/1.1 " << status << "\n" <<
		"Content-Type: text/html\n" <<
		"Content-Length: " << index_html.length() << "\n" <<
		"AutoIndex: " << folder << "\n" <<
		location_header <<
		"\n"
	);

	// TESTING_print_response(response_header, index_html);
    std::cout << response_header + index_html << std::endl;

	return (response_header + index_html);
}

int main(int ac, char **av)
{
    if (ac != 5)
    {
        std::cout << "need 4 args: std::string target, "
            << "std::string location_root, "
            << "std::string location_name, "
            << "std::string folder";
    }

    create_autoindex_response(av[1], av[2], av[3], av[4]);

    return(0);
}