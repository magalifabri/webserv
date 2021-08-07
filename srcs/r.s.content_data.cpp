#include "request.struct.hpp"

void request::collect_body(const char *data, const size_t &data_size)
{
	/* if there is still data is headers_data, this data is part of the body */
	if (headers_data.empty() == false)
		copy_from_headers_data();
	else if (data_contains_headers == false)
		append_requests_data(data, data_size);
	
	if (content_nbytes_received == content_length)
	{
		std::cout << CYAN "body data fully received" RESET << std::endl; // TESTING
		request_received = true;
	}
	// std::cout << "content_nbytes_received: " << content_nbytes_received << std::endl; // TESTING
}



void request::copy_from_headers_data(void)
{
	if (headers_data.size() > content_length)
		body_data.insert(body_data.end(), headers_data.begin(), headers_data.begin() + content_length);
	else
		body_data.insert(body_data.end(), headers_data.begin(), headers_data.end());
	headers_data.clear();
	content_nbytes_received += body_data.size();
}



void request::append_requests_data(const char *data, const size_t &data_size)
{
	if (content_nbytes_received + data_size > content_length)
	{
		body_data.insert(body_data.end(), data, data + (content_length - content_nbytes_received));
		content_nbytes_received += (content_length - content_nbytes_received);
		// std::cout << (content_length - content_nbytes_received) << " of " << data_size << " bytes added to body" << std::endl; // TESTING
	}
	else
	{
		body_data.insert(body_data.end(), data, data + data_size);
		content_nbytes_received += data_size;
		// std::cout << data_size << " bytes added to body" << std::endl; // TESTING
	}
}
