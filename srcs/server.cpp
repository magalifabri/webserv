#include "webserv.hpp"

int	main(int argc, char** argv)
{
	webserv webserv;
	try
	{
		webserv.set_path_to_config(argv);
		webserv.init_config_server_blocks(argc, argv);
		run_servers(webserv);
		webserv.servers.clear();
	}
	catch(const std::exception& e)
	{
		std::cerr << "Error: " << e.what() << '\n';
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}



void run_servers(webserv& webserv)
{
	std::vector<int> listen_sds = create_listen_sds(webserv);
	while (true)
	{
		webserv.read_set = webserv.master_set;
		webserv.write_set = webserv.master_set;

		int n_selected_sds = select(webserv.max_sd_in_set + 1,
			&webserv.read_set, &webserv.write_set, NULL, &webserv.timeout);
		if (n_selected_sds == -1)
			throw std::runtime_error("select() failed");
		else if (n_selected_sds == 0)
			std::cout << "select() timed out" << std::endl;
		
		if (handle_selected_sds(webserv, listen_sds) == false)
			break ;
	}
	
	close_sds(webserv);
}



bool handle_selected_sds(webserv& webserv, std::vector<int> listen_sds)
{
	for (int sd = 0; sd <= webserv.max_sd_in_set; sd++)
	{
		if (FD_ISSET(sd, &webserv.read_set))
		{
			/* Check if current checked sd is a listening socket. */
			for (std::vector<int>::iterator it = listen_sds.begin(); it != listen_sds.end(); it++)
				if (sd == (*it))
					return (accept_new_connection(webserv, (*it)));

			/* It's not a listening socket, so an existing connection is sending a request. */
			if (webserv.request_map[sd].in_use == false)
			{
				std::cout << std::endl << BOLD "New request coming in on sd " << sd << RESET << std::endl;
				webserv.request_map[sd] = request(webserv.client_ip_map[sd]);
			}
			else
				std::cout << std::endl << BOLD "Data for existing request coming in on sd " << sd << RESET << std::endl;

			connect_to_existing_sd(webserv, sd, webserv.request_map[sd]);
		}
	}
	return (true);
}



void close_sds(webserv& webserv)
{
	for (int sd = 0; sd <= webserv.max_sd_in_set ; ++sd)
		if (FD_ISSET(sd, &webserv.master_set))
			close(sd);
}
