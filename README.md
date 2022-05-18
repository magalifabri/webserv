# WEBSERV

Exercise from coding school 19 to write an HTTP web server from scratch with C++.

### status: completed & validated
:white_check_mark: : 100/100

## HOW TO USE
1. clone repo
2. `cd` into repo
3. `make`
4. `./server <config file (optional)>`
5. go to localhost:8080 (or a different port depending on the config file used) in the browser (Chrome is suggested) to interact with the server, or use curl.

## FUNCTIONALITY
- I/O multiplexing with select()
- file uploading with POST and PUT method requests and the webform (POST) on the upload.html page
- file deletion with DELETE method request
- autoindex pages with links to content
- set up a custom configuration in which you can define the following (see config/config.conf for more info):
    - 1+ virtual servers
	- hostname & port
	- 1+ server names
	- root
	- default file to search for when requesting a directory
	- directory listing (on or off)
	- maximum accepted request body size
	- location of default error pages
	- location where uploads should be saved
	- server locations (routes to a directory)
	- accepted HTTP methods
	- HTTP redirection
- implemented HTTP methods:
    - GET
    - HEAD
    - POST
    - PUT
    - DELETE
- set maximum length of request URI target (in path: includes/request.struct.hpp)

## RESOURCES
- [vid: Program your own web server in C. (sockets)](https://www.youtube.com/watch?v=esXw4bdaZkc)
- [vid: How one thread listens to many sockets with select in C.](https://www.youtube.com/watch?v=Y6pFtgRdUts)
- [Everything you need to know to Build a simple HTTP server from scratch](https://medium.com/from-the-scratch/http-server-what-do-you-need-to-know-to-build-a-simple-http-server-from-scratch-d1ef8945e4fa)
- [Beej's Guide to Network Programming: 7.3 select()—Synchronous I/O Multiplexing, Old School](https://beej.us/guide/bgnet/html/#select)
- [Example: Nonblocking I/O and select()](https://www.ibm.com/docs/en/i/7.1?topic=designs-example-nonblocking-io-select)
- [403 Forbidden vs 401 Unauthorized HTTP responses](https://stackoverflow.com/questions/3297048/403-forbidden-vs-401-unauthorized-http-responses)
- [Fastest way to check if a file exist using standard C++/C++11/C?](https://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c)
- [Checking if path is a directory or a file](https://stackoverflow.com/questions/4553012/checking-if-a-file-is-a-directory-or-just-a-file)
- [NginX: Root Directory and Index Files](https://docs.nginx.com/nginx/admin-guide/web-server/serving-static-content/#root-directory-and-index-files)
- [NginX: Directives](http://nginx.org/en/docs/http/ngx_http_core_module.html#directives)
- [NginX: Return Directive](http://nginx.org/en/docs/http/ngx_http_rewrite_module.html#return)
- [HTTP Redirections](https://developer.mozilla.org/en-US/docs/Web/HTTP/Redirections)
- [CGI Environment Variables](https://www.ibm.com/docs/en/netcoolomnibus/8.1?topic=scripts-environment-variables-in-cgi-script)

## NOTES
### POST
**Command**: `curl -X POST localhost:8080/subdir_POST/some/path -d "Hello There"`

**Configurations**:
```
location /subdir_POST {
	accepted_methods POST;
	upload_path www/testing/subdir_upload;
}
```
**Short**: A file named *upload[number]* with the content *“Hello There”* will be created at www/testing/subdir_upload/.

**Long**: All requests with a URI that start with /subdir_POST will be caught by this location, so this includes the URI specified in the curl command. The */some/path* part of the URI is ignored.

If the POST method is accepted, which in this case it is, a new file will be created. This file will be created at the path specified with the upload_path directive. If no directive is given, the /tmp/uploads/ path is used as a fallback.

The file will be named *upload[number]*, where [number] is replaced by the lowest number of which a file with that name does not yet exist in the directory.

The data specified with the -d flag is written to the file.

### PUT
**Command**: `curl -X PUT localhost:8080/subdir_PUT/path/to/newfile -d "Hello There"`

**Configurations**:
```
location /subdir_PUT {
	accepted_methods PUT;
}
```

A file named *newfile* with the content *“Hello There”* is created at /subdir_PUT/path/to/. If it already exists, it is overwritten. Missing intermediate directories (/path/to/) are created.

**Errors**
- A URI that ends with a slash, and thus doesn’t specify a file, triggers a 400 error.
- When the URI contains a directory with the same name as an already existing file, mkdir (man 2 mkdir) returns -1 “File exists” error, triggering a 500 error.

### GIT COMMIT SYMBOLISM
- `^` : general update
- `+` : added/implemented
- `-` : removed
- `++` : improved
- `x => y` : x changed into/replaced by y
