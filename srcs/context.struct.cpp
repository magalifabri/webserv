#include "context.struct.hpp"

context::context(void) : root(""), autoindex(false), autoindex_set(false), client_max_body_size(-1), upload_path(""), return_code(-1), redirect("") {
	this->error_pages[400] = "";
	this->error_pages[403] = "";
	this->error_pages[404] = "";
	this->error_pages[405] = "";
	this->error_pages[413] = "";
	this->error_pages[500] = "";
	this->error_pages[505] = "";
}

context::~context(void) {}

server::server(void) : context(), default_server(false) {
	this->status_codes[300] = "300 Multiple Choice"; // The choices are listed in an HTML page in the body and the user clicks on one to select it.
	this->status_codes[301] = "301 Moved Permanently"; // Use: Reorganization of a Web site.
	this->status_codes[302] = "302 Found"; // The Web page is temporarily unavailable for unforeseen reasons.
	this->status_codes[303] = "303 See Other"; // Used to redirect after a PUT or a POST, so that refreshing the result page doesn't re-trigger the operation.
	this->status_codes[304] = "304 Not Modified"; // Sent for revalidated conditional requests. Indicates that the cached response is still fresh and can be used.
	this->status_codes[307] = "307 Temporary Redirect"; // The Web page is temporarily unavailable for unforeseen reasons. Better than 302 when non-GET operations are available on the site.
	this->status_codes[308] = "308 Permanent Redirect"; // Use: Reorganization of a Web site, with non-GET links/operations.
	this->status_codes[400] = "400 Bad Request"; // The server could not understand the request due to invalid syntax.
	this->status_codes[403] = "403 Forbidden"; // The client does not have access rights to the content; that is, it is unauthorized, so the server is refusing to give the requested resource.
	this->status_codes[404] = "404 Not Found"; // The server can not find the requested resource.
	this->status_codes[405] = "405 Method Not Allowed"; // The request method is known by the server but is not supported by the target resource.
	this->status_codes[413] = "413 Request Entity Too Large";
	this->status_codes[500] = "500 Internal Server Error"; // The server has encountered a situation it doesn't know how to handle.
	this->status_codes[501] = "501 Not Implemented"; // The request method is not supported by the server and cannot be handled.
	this->status_codes[505] = "505 HTTP Version Not Supported";
}

server::server(server const& src) { *this = src; }

server& server::operator=(server const& src) {
	if (this != &src)
	{
		this->root = src.root;
		this->index = src.index;
		this->accepted_methods = src.accepted_methods;
		this->autoindex = src.autoindex;
		this->autoindex_set = src.autoindex_set;
		this->client_max_body_size = src.client_max_body_size;
		this->error_pages = src.error_pages;
		this->upload_path = src.upload_path;
		this->return_code = src.return_code;
		this->redirect = src.redirect;
		this->host = src.host;
		this->port = src.port;
		this->default_server = src.default_server;
		this->names = src.names;
		this->locations = src.locations;
		this->status_codes = src.status_codes;
	}
	return *this;
}

server::~server(void) {}

location::location(void) {}

location::location(struct server& s) : context(), name(""), cgi(false), cgi_ext(""), cgi_pass("") {
	this->server = &s;
}

location::location(struct location const& src) { *this = src; }

location& location::operator=(location const& src) {
	if (this != &src)
	{
		this->root = src.root;
		this->index = src.index;
		this->accepted_methods = src.accepted_methods;
		this->autoindex = src.autoindex;
		this->autoindex_set = src.autoindex_set;
		this->client_max_body_size = src.client_max_body_size;
		this->error_pages = src.error_pages;
		this->upload_path = src.upload_path;
		this->return_code = src.return_code;
		this->redirect = src.redirect;
		this->server = src.server;
		this->name = src.name;
		this->cgi = src.cgi;
		this->cgi_ext = src.cgi_ext;
		this->cgi_pass = src.cgi_pass;
	}
	return *this;
}

location::~location(void) {}
