#include "webserv.hpp"

bool send_response(
	webserv &webserv, const int &sd, const std::string &response, request &request)
{
	/* Confirm that sd is ready for writing to */
	if (FD_ISSET(sd, &webserv.write_set))
	{
		/* If request method is HEAD, only send the header section */
		/* Send response to the client. */

		int nbytes_sent = sendall(
			sd, &response[0], get_response_length(response, request));
		if (nbytes_sent == -1)
		{
			perror("Error: send()"); // TESTING
			return (false);
		}
		else if (nbytes_sent == 0)
		{
			std::cerr << "Error: send() returned 0\n";
			return (false);
		}
		webserv.response_counter++; // TESTING
		std::cout << "response n#" << webserv.response_counter << " of " << nbytes_sent << " bytes sent\n"; // TESTING
	}
	return (true);
}

int sendall(const int &sd, const char *buf, const int &len)
{
    int total_bytes_sent = 0;
    int bytes_left = len;
    int nbytes_sent;

	/* If only one send() it required we can avoid the usleep() */
	nbytes_sent = send(sd, buf + total_bytes_sent, bytes_left, 0);
	if (nbytes_sent < 0)
		return (nbytes_sent);
	total_bytes_sent += nbytes_sent;
	bytes_left -= nbytes_sent;

    while (total_bytes_sent < len)
	{
		/* usleep() to avoid send() errorring with EWOULDBLOCK because the
		client can't keep up */
		// usleep(100000);
		usleep(10000);
        nbytes_sent = send(sd, buf + total_bytes_sent, bytes_left, 0);
		if (nbytes_sent < 0)
			return (nbytes_sent);
        total_bytes_sent += nbytes_sent;
        bytes_left -= nbytes_sent;
    }

    return (total_bytes_sent);
}

size_t get_response_length(const std::string &response, request &request)
{
	if (request.headers_map["method"] == "HEAD")
		return (response.find("\n\n") + 2);
	return (response.length());
}
