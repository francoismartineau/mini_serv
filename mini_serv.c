#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_CLIENTS 1024

void exit_error(char *msg)
{
    write(2, msg, strlen(msg));
    exit(1);
}

void fatal_error()
{
    exit_error("Fatal error\n");
}


int main(int argc, char** argv)
{
    if (argc != 2)
        exit_error("Wrong number of arguments\n");
	int acptsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(acptsock == -1)
		fatal_error();
	fcntl(acptsock, F_SETFL, O_NONBLOCK);

	struct sockaddr_in bindaddr;
	bindaddr.sin_family = AF_INET;
	bindaddr.sin_addr.s_addr = INADDR_ANY;
	bindaddr.sin_port = htons(atoi(argv[1]));

	if(bind(acptsock, (struct sockaddr*)&bindaddr, sizeof(bindaddr)) == -1)
		fatal_error();
	if(listen(acptsock, 10) == -1)
		fatal_error();

	size_t clients_count = 0;
	int clients[MAX_CLIENTS] = {0};

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(acptsock, &readfds);

	int maxfd = acptsock;

	while(1)
	{
		fd_set readfds_copy = readfds;
		if(select(maxfd + 1, &readfds_copy, 0, 0, 0) == -1)
            fatal_error();
		if(FD_ISSET(acptsock, &readfds_copy))
		{
			int clntsock = accept(acptsock, 0, 0);
			if(clntsock == -1)
                fatal_error();
			clients[clients_count++] = clntsock;
			FD_SET(clntsock, &readfds);
			if(clntsock > maxfd)
				maxfd = clntsock;
		}
		for(size_t i = 0; i < clients_count; ++i)
		{
			if(FD_ISSET(clients[i], &readfds_copy))
			{
				char buf[1024];
				ssize_t bytes = recv(clients[i], buf, sizeof(buf), 0);
				if(bytes == -1)
					fatal_error();
				if(bytes == 0)
				{
					close(clients[i]);
					FD_CLR(clients[i], &readfds);
					for(size_t j = i; j < clients_count - 1; ++j)
						clients[j] = clients[j + 1];
					--clients_count;
					--i;
					continue;
				}
				for(size_t j = 0; j < clients_count; ++j)
					if(clients[j] != clients[i])
						if(send(clients[j], buf, bytes, 0) == -1)
							fatal_error();
			}
		}
	}

	return 0;
}