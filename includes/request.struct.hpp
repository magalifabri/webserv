#ifndef REQUEST_STRUCT_HPP
#define REQUEST_STRUCT_HPP

#define URI_MAX_LENGTH 300

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN "\033[0;36m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

#include "context.struct.hpp" // server struct, location struct
#include "webserv.hpp"
void	TESTING_print_vector_char(std::vector<char> vector); // TESTING

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>

struct webserv;

struct request
{
	request(void);
	request(std::string client_ip);

    void process_data(const char *data, const size_t &data_size, int sd, webserv &webserv);
    void parse_first_line(void);
    void set_method(const std::string &request, size_t &current_pos);
    void set_target(const std::string &request, size_t &current_pos);
    void set_protocol_version(const std::string &request, size_t &current_pos);
    
    void parse_headers(void);
    
    void collect_body(const char *data, const size_t &data_size);
    void copy_from_headers_data(void);
    void append_requests_data(const char *data, const size_t &data_size);

    bool collect_body_chunked(const char *data, const size_t &data_size);
    void parse_chunked_data(void);
    unsigned int convert_hex_str_to_int(std::string chunk_size_hex);

    void parse_URI_query(void);

    bool data_contains_headers;
    std::vector<char> headers_data;
    std::map<std::string, std::string> headers_map;
    std::vector<char> body_data;
    std::map<std::string, std::string> URI_query_map;
    
    /*
    in_use:
    true: request has not yet been completed and should not be overwritten with a fresh request
    
    - initialised to false in integrate_new_connection()
    - set to true when the request struct is initialised in handle_selected_sds()
    - set back to false in connect_to_existing_sd(), after sending the response
    */
    bool in_use;
    unsigned int content_length;
    unsigned int content_nbytes_received;
    bool first_line_received;
    bool headers_received;
    bool request_received;
    bool response_sent;

	std::string	client_ip;
	//
	std::string match;
	std::string	script_name;
	std::string	path_info;
	std::string	query_str;
	//
    
    server conf;
    location location;
};

server&		select_server(int sd, std::vector<server> servers,
				std::map<size_t, size_t> port_map, std::string host);
location&	select_location(server conf, std::string URI, std::string method);

#endif
