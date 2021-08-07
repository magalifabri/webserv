#include "webserv.hpp"

void	execute_cgi(webserv& webserv, request& request, location& location)
{
	init_fds(webserv);

	std::string input = get_input(request, location.root);
	// std::cout << "input: " << input << '\n'; // TESTING

	std::string document_root = webserv.cwd;
	if (location.root[0] != '/')
		document_root += location.root;
	else
		document_root = location.root;
	// std::cout << "document_root: " << document_root << '\n'; // TESTING

	char** args = get_args(webserv.cwd, location.cgi_pass, document_root + request.headers_map["target"]);
	char** envp = get_env(request, SSTR(input.length()), document_root);

	write(webserv.fdin, input.c_str(), input.length());
	lseek(webserv.fdin, 0, SEEK_SET);

	std::string response;
	pid_t pid = fork();
	if (pid < 0)
	{
		close_fds(webserv);
		delete_arrays(args, envp);
		throw std::runtime_error("fork() failed");
	}
	else if (!pid)
	{
		/* change directory to document root */
		if (chdir((webserv.cwd + location.root).c_str()) < 0)
		{
			std::cerr << "Error: chdir() failed\n";
			exit(1);
		}

		dup2(webserv.fdin, STDIN_FILENO);
		dup2(webserv.fdout, STDOUT_FILENO);

		if (execve(*args, args, envp) < 0)
		{
			std::cerr << "Error: execve() failed\n";
			exit(1);
		}
	}
	else
	{
		char buffer[BUFFER_SIZE] = {0};

		waitpid(-1, NULL, 0);
		lseek(webserv.fdout, 0, SEEK_SET);

		int	ret = 1;
		while (ret > 0)
		{
			memset(buffer, 0, BUFFER_SIZE);
			ret = read(webserv.fdout, buffer, BUFFER_SIZE - 1);
			response += buffer;
		}
	}

	close_fds(webserv);
	delete_arrays(args, envp);

	edit_response_headers(request, response);
}

void	edit_response_headers(request& request, std::string response)
{
	std::cout << RED "RESPONSE\n" RESET << response << '\n'; // TESTING

	std::string headers = response.substr(0, response.find("\r\n\r\n") + 4);
	std::string body = response.substr(response.find("\r\n\r\n") + 4);

	size_t pos = headers.find("Status:");

	if (headers.find("HTTP/1.1") == std::string::npos && pos == std::string::npos)
		headers.replace(0, 0, "HTTP/1.1 200 OK\n");

	if (headers.find("HTTP/1.1") == std::string::npos && pos != std::string::npos)
		headers.replace(pos, std::string("Status:").length(), "HTTP/1.1");

	if (headers.find("Content-Length") == std::string::npos)
		headers.replace(headers.find("Content-"), 0, SSTR("Content-Length: " << body.length() << "\n"));

	// std::cout << BOLD << GREEN << headers << RESET << '\n'; // TESTING

	request.response_header = headers;
	request.response_body = body;
}
