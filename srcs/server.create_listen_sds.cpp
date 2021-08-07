#include "webserv.hpp"

std::vector<int> create_listen_sds(webserv &webserv)
{
	int listen_sd;
	std::vector<int> listen_sds;
	for (std::vector<server>::iterator it = webserv.servers.begin(); it != webserv.servers.end(); it++)
	{
		for (size_t i = 0; i < (*it).port.size(); i++)
		{
			bool listening = false;
			std::map<size_t, size_t>::iterator it2 = webserv.port_map.begin();
			for (; it2 != webserv.port_map.end(); it2++)
				if ((listening = (it2->second == (*it).port[i])))
					break ;
			if (listening)
				continue ;
			std::cout << "\nRetrieving sd for port " << (*it).port[i] << '\n';
			listen_sd = initialise_listen_sd((*it).port[i]);
			listen_sds.push_back(listen_sd);
			webserv.port_map[listen_sd] = (*it).port[i];
			// std::cout << "\nStoring config for port: " << server_map[listen_sd].port[i] << '\n';
			
			/* Add the socket for listening to master_set. */
			FD_SET(listen_sd, &webserv.master_set);
			/* Keep track of the sd with the highest number value. */
			if (listen_sd > webserv.max_sd_in_set)
				webserv.max_sd_in_set = listen_sd;
		}
	}

	std::cout << "\nListening on " << listen_sds.size() << " port(s)\n";

	return (listen_sds);
}



int initialise_listen_sd(const int &port)
{
	/* create an PF_INET stream socket to receive incoming connections on */
	int listen_sd = socket(PF_INET, SOCK_STREAM, 0);
	if (listen_sd == -1)
		throw std::runtime_error("socket() failed");

	/* allow socket descriptor to be reuseable */
	int on = 1;
	if (setsockopt(listen_sd, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on)) < 0)
		throw std::runtime_error("setsockopt() failed");

	/* set the socket to be non-blocking */
	if (fcntl(listen_sd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl() failed");

	/* bind the socket */
	struct sockaddr_in server_adrr;
	memset(&server_adrr, 0, sizeof(server_adrr));
	server_adrr.sin_family = AF_INET;
	server_adrr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_adrr.sin_port = htons(port);
	memset(server_adrr.sin_zero, '\0', sizeof(server_adrr.sin_zero));
	if (bind(listen_sd, (struct sockaddr*)&server_adrr, sizeof(server_adrr)) == -1)
		throw std::runtime_error("bind() failed");

	/* set the listen back log */
	if (listen(listen_sd, BACKLOG) == -1)
		throw std::runtime_error("listen() failed");
	return (listen_sd);
}
