#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <fcntl.h>


// les ids changent quand quelqu'un se déconnecte
// max msg len?

void error(char *msg)
{
	write(2, msg, strlen(msg));
	exit(1);
}

void fatal_error()
{
	error("Fatal error\n");
}

void broadcast(char *msg, int *clients, size_t clients_count, size_t except)
{
	for (size_t i = 0; i < clients_count; ++i)
	{
		if (i == except)
			continue;
		if (send(clients[i], msg, strlen(msg), 0) == -1)
			fatal_error();
	}
	printf("%s\n", msg);
}

int main(int argc, char *argv[])
{
	if (argc < 2)
		error("Wrong number of arguments\n");

	int sockfd, connfd;
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
		fatal_error();
	fcntl(sockfd, F_SETFL, O_NONBLOCK);

	struct sockaddr_in servaddr; 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(atoi(argv[1])); 
  
	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) == -1
		|| listen(sockfd, 10) == -1)
		fatal_error();

	size_t clients_count = 0;
	int clients[1024] = {0};

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	
	int maxfd = sockfd;

	char buf[1024];
	char buf2[1024];
	while (1)
	{
		fd_set readfds_copy = readfds;
		if (select(maxfd + 1, &readfds_copy, 0, 0, 0) == -1)
			fatal_error();
		if (FD_ISSET(sockfd, &readfds_copy))
		{
			connfd = accept(sockfd, 0, 0);
			if (connfd == -1)
				fatal_error();
			sprintf(buf2, "server: client %lu just arrived\n", clients_count);
			broadcast(buf2, clients, clients_count, -1);
			clients[clients_count++] = connfd;
			FD_SET(connfd, &readfds);
			maxfd = maxfd < connfd ? connfd : maxfd;
		}
		for (size_t i = 0; i < clients_count; ++i)
		{
			if (!FD_ISSET(clients[i], &readfds_copy))
				continue;
			ssize_t bytes = recv(clients[i], buf, sizeof(buf), 0);
			if (bytes == -1)
				fatal_error();
			if (bytes == 0)
			{
				close(clients[i]);
				FD_CLR(clients[i], &readfds);
				for (size_t j = i; j < clients_count - 1; ++j)
					clients[j] = clients[j + 1];
				--clients_count;
				sprintf(buf2, "server: client %lu just left\n", i);
				--i;
				broadcast(buf2, clients, clients_count, -1);
				continue;
			}
			buf[bytes] = '\0';
			sprintf(buf2, "client %lu: %s", i, buf);
			broadcast(buf2, clients, clients_count, i);
		}
	}
}
