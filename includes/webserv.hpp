#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "context.struct.hpp"
# include "request.struct.hpp"

# include <sys/socket.h>
# include <netinet/in.h> // struct in_addr
# include <sys/stat.h> // stat
# include <unistd.h>
# include <dirent.h> // opendir, readdir
# include <stdio.h> // perror
# include <errno.h>
# include <fcntl.h>

# include <iostream>
# include <fstream>
# include <string>
# include <sstream> // stringstream

# include <algorithm> // find, search
# include <vector>
# include <map>

# define BACKLOG 1000000
# define CONFIG_DIR "config/"
# define ERROR_DIR "www/errors/"
# define BUFFER_SIZE 1024

# define RED "\033[0;31m"
# define GREEN "\033[0;32m"
# define YELLOW "\033[0;33m"
# define BLUE "\033[0;34m"
# define MAGENTA "\033[0;35m"
# define CYAN "\033[0;36m"
# define BOLD "\033[1m"
# define RESET "\033[0m"

# define SSTR(x) static_cast<std::ostringstream&>((std::ostringstream() << std::dec << x)).str() // convert ... to string

struct request;

struct webserv
{
	webserv(void);

	std::string cwd;
	std::string path_to_config;
	std::vector<server> servers;
	std::map<size_t, size_t> port_map;
	std::map<size_t, std::string> client_ip_map;
	struct fd_set master_set, read_set, write_set;
	int max_sd_in_set;
	int response_counter; // TESTING
	struct timeval timeout;
	std::map<int, request> request_map;
	
	void set_path_to_config(char **argv);
	void init_config_server_blocks(int argc, char **argv);

	int stdin_save;
	int stdout_save;
	FILE* fin;
	FILE* fout;
	long fdin;
	long fdout;
};

/* create_autoindex_response.cpp */
std::string		create_autoindex_response(
				const location &location, const std::string &folder);
std::vector<char>	create_html_template_vector(const std::string URI);
void			insert_string_into_template(std::vector<char> &html_template,
				const std::string &insert, const std::string location);

/* create_get_form_response.cpp */
std::string     create_get_form_response(request &request);

/* execute cgi utils */
void			init_fds(webserv& webserv);
std::string		get_input(request& request, std::string document_root);
char**			get_args(std::string cwd, std::string cgi_pass, std::string target);
void			close_fds(webserv& webserv);
void			delete_arrays(char** args, char** envp);

/* execute cgi */
std::string		execute_cgi(webserv& webserv, request& request, location& location);
void			edit_response_headers(std::string* response);

/* get env */
char**			get_env(webserv& webserv, request& request, std::string const& content_length);
std::map<std::string, std::string>	add_request_headers(request& request, std::map<std::string, std::string> env);
std::string		format_http_header(std::string const& name);
char**			to_envp(std::map<std::string, std::string> env);

/* handle_GET_HEAD.cpp */
std::string		handle_GET_or_HEAD_request(webserv& webserv, request &request);

/* handle_POST_PUT_DEL.cpp */
std::string		handle_POST_request(webserv& webserv, location location, request request);
std::string		handle_PUT_request(const location &location, request &request);
std::string		handle_DELETE_request(const location &location, request &request);

/* handle_webform_POST.cpp */
std::string handle_webform_POST(const location &location, request &request);

/* parse_config */
typedef	void(*functionPtr)(webserv& webserv, context* cxt, std::vector<std::string> values);

void	parse_config(webserv& webserv, std::string path);

void	parse_server(webserv& webserv, std::string str);
void	parse_locations(webserv& webserv, server& server, std::vector<std::string> locations);

void	set_listen(webserv& webserv, context* cxt, std::vector<std::string> values);
void	set_server_names(webserv& webserv, context* cxt, std::vector<std::string> values);
void	set_error_page(webserv& webserv, context* cxt, std::vector<std::string> values);
void	set_root(webserv& webserv, context* cxt, std::vector<std::string> values);
void	set_index(webserv& webserv, context* cxt, std::vector<std::string> values);
void	set_autoindex(webserv& webserv, context* cxt, std::vector<std::string> values);
void	set_client_max_body_size(webserv& webserv, context* cxt, std::vector<std::string> values);
void	set_accepted_methods(webserv& webserv, context* cxt, std::vector<std::string> values);
void	set_cgi(webserv& webserv, context* cxt, std::vector<std::string> values);
void	set_upload_enable(webserv& webserv, context* cxt, std::vector<std::string> values);
void	set_upload_path(webserv& webserv, context* cxt, std::vector<std::string> values);
void	set_return(webserv& webserv, context* cxt, std::vector<std::string> values);

void	edit_server(webserv& webserv, server& server);
void	edit_location(server& server, location& location);


/* parse_config_utils */
std::string					conform_path(std::string& param, bool trailing_slash);
size_t						param_to_u_int(std::string value);
std::vector<std::string>	get_params(std::string params);

/* process_request.cpp */
std::string		process_request(webserv& webserv, request &request);
location&		get_location(server conf, std::string* URI);
bool		    check_location(location& l, std::string* URI, bool is_dir);

/* response_part_creation.cpp */
std::string     get_content_type(const std::string &status, const std::string &URI);
std::string		get_response_body(const std::string &html_page_URI);

/* response_page_creation.cpp */
std::string     create_error_response(const int &error_code, location &location);
std::string		create_2xx_response_page(
				const std::string &status, const std::string &location_header = "");

/* server.cpp */
void 			run_servers(webserv &webserv);
bool			handle_selected_sds(webserv &webserv, std::vector<int> listen_sds);
void			close_sds(webserv &webserv);

/* server.create_listen_sds.cpp */
std::vector<int> create_listen_sds(webserv &webserv);
int				initialise_listen_sd(const int &port);

/* server.handle_existing_sd.cpp */
void			connect_to_existing_sd(webserv &webserv, const int &sd, request &request);
bool			receive_request(webserv &webserv, const int &sd, request &request);
bool			try_to_process_data(webserv &webserv, request &request, char *request_buffer, int &nbytes, const int &sd);
void			f_close_sd(webserv &webserv, const int &sd);

/* server.handle_new_sd.cpp */
bool 			accept_new_connection(webserv &webserv, int &listen_sd);
void			integrate_new_connection(webserv &webserv, const int &new_sd, const sockaddr_in &client_addr, const int &listen_sd);

/* server.send_response.cpp */
bool			send_response(webserv &webserv, const int &sd, const std::string &response, request &request);
int				sendall(const int &sd, const char *buf, const int &len);
size_t			get_response_length(const std::string &response, request &request);

/* select */
bool			select_index(const location& location, std::string* URI);

/* utils */
bool			is_file(std::string path);
bool			is_directory(std::string path);
std::string		ft_inet_ntoa(struct in_addr in);
bool			mkdir_p(std::string path);



/* tests */
void            TESTING_print_parsed_request(request request);
void            TESTING_print_ready_sockets(
				int max_sd_in_set, fd_set read_set, fd_set write_set);
void            TESTING_print_vector_char(std::vector<char> vector);
void			TESTING_print_vector_string(std::vector<std::string> content_list);
void	        TESTING_print_servers(std::vector<server> servers);
void			TESTING_print_response(std::string header, std::string body);

#endif
