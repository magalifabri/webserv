#include "webserv.hpp"

void connect_to_existing_sd(webserv &webserv, const int &sd, request &request)
{
	bool close_sd = receive_request(webserv, sd, request);
	if (request.request_received == true &&
		request.response_sent == false)
	{
		std::string response;
		try
		{
			response = process_request(webserv, request);
		}
		catch(int error_code)
		{
			response = create_error_response(error_code, request.location);
		}
		
		if (send_response(webserv, sd, response, request) == false)
			close_sd = true;
		
		request.in_use = false;
	}
	
	if (close_sd == true)
		f_close_sd(webserv, sd);
}



bool receive_request(webserv &webserv, const int &sd, request &request)
{
	bool close_sd = false;
	/* Receive data on this connection until recv() fails */
	while (true)
	{
		char request_buffer[500000] = {0};
		int nbytes = recv(sd, request_buffer, sizeof(request_buffer), 0);
		/* To not rush the client to send more data, we have a little nap */
		// usleep(1000);
		if (nbytes <= 0)
		{
			if (nbytes == -1)
				perror("recv() returned -1"); // TESTING
			else if (nbytes == 0)
			{
				std::cerr << "recv() returned 0: Client closed connection (sd " << sd << ")" << std::endl ;
				close_sd = true;
			}
			break ;
		}
		std::cout << nbytes << " bytes received / "
		<< request.headers_data.size() + request.body_data.size() + nbytes << " in total" << std::endl; // TESTING

		if (try_to_process_data(webserv, request, request_buffer, nbytes, sd) == false)
			return (true);
	}
	return (close_sd);
}



bool try_to_process_data(webserv &webserv, request &request, char *request_buffer, int &nbytes, const int &sd)
{
	try
	{
		request.process_data(request_buffer, nbytes, sd, webserv);
		// TESTING_print_parsed_request(request);
	}
	// catch (const char *param)
	catch (std::string status) // floor
	{
		/* create error response and send it */
		// std::string status(param);
		std::string response = "HTTP/1.1 " + status + "\n\n";
		TESTING_print_response(response, "");
		send_response(webserv, sd, response, request);
		return (false);
	}

	return (true);
}



void f_close_sd(webserv &webserv, const int &sd)
{
	std::cout << CYAN "closing sd " << sd << RESET << std::endl; // TESTING
	close(sd);
	FD_CLR(sd, &webserv.master_set);
	/* If sd is the current max, set a new max. */
	if (sd == webserv.max_sd_in_set)
		while (FD_ISSET(webserv.max_sd_in_set, &webserv.master_set) == false)
			webserv.max_sd_in_set--;
}
