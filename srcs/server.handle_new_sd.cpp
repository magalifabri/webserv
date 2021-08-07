#include "webserv.hpp"

bool accept_new_connection(webserv &webserv, int &listen_sd)
{
	std::cout << std::endl << BOLD "Listening sd " << listen_sd << " is readable" << RESET << std::endl; // TESTING
	
	sockaddr_in client_addr;
	socklen_t size = sizeof(client_addr);
	while (true)
	{
		int new_sd = accept(listen_sd, (sockaddr*)&client_addr, &size);
		std::cout << std::endl << BOLD "Accepting new sd " << new_sd << RESET << std::endl; // TESTING
		if (new_sd == -1)
		{
			/* if accept() fails with EWOULDBLOCK, then we have accepted all of them */
			if (errno != EWOULDBLOCK)
			{
				std::cerr << "Error: accept() failed\n";
				return (false);
			}
			break ;
		}
		integrate_new_connection(webserv, new_sd, client_addr, listen_sd);
	}
	return (true);
}



void integrate_new_connection(
	webserv &webserv, const int &new_sd, const sockaddr_in &client_addr, const int &listen_sd)
{
	/* Add the new incoming connection to the list of sockets that we're watching, (master_set). */
	std::cout << "New incoming connection: sd " << new_sd << "\n"; // TESTING
	FD_SET(new_sd, &webserv.master_set);
	if (new_sd > webserv.max_sd_in_set)
		webserv.max_sd_in_set = new_sd;

	std::string client_ip = ft_inet_ntoa(client_addr.sin_addr);
	std::cout << "\nclient IP: " << client_ip << '\n'; // TESTING
	webserv.port_map[new_sd] = webserv.port_map[listen_sd];
	webserv.client_ip_map[new_sd] = client_ip;

	webserv.request_map[new_sd].in_use = false;
}
