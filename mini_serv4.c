#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <fcntl.h>


// max msg len?
// pas de messages sans new line? est-ce qui faut les rejoindre quand un newline est donné?

void error(char *msg)
{
	write(2, msg, strlen(msg));
	exit(1);
}

void fatal_error()
{
	error("Fatal error\n");
}

struct client {
	int fd;
	int id;
};

void broadcast(char *msg, struct client *clients, size_t clients_count, size_t except)
{
	for (size_t i = 0; i < clients_count; ++i)
	{
		if (i == except)
			continue;
		if (send(clients[i].fd, msg, strlen(msg), 0) == -1)
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
  
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) == -1
		|| listen(sockfd, 10) == -1)
		fatal_error();

	size_t clients_count = 0;
	struct client clients[1024] = {0};

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	
	int maxfd = sockfd;
	int maxid = -1;

	char buf[2097152];
	char buf2[2097152];
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
			sprintf(buf2, "server: client %d just arrived\n", ++maxid);
			broadcast(buf2, clients, clients_count, -1);
			clients[clients_count].fd = connfd;
			clients[clients_count].id = maxid;
			++clients_count;
			FD_SET(connfd, &readfds);
			maxfd = maxfd < connfd ? connfd : maxfd;
		}
		for (size_t i = 0; i < clients_count; ++i)
		{
			if (!FD_ISSET(clients[i].fd, &readfds_copy))
				continue;
			ssize_t bytes = recv(clients[i].fd, buf, sizeof(buf), 0);
			if (bytes == -1)
				fatal_error();
			if (bytes == 0)
			{
				close(clients[i].fd);
				FD_CLR(clients[i].fd, &readfds);
				for (size_t j = i; j < clients_count - 1; ++j)
					clients[j] = clients[j + 1];
				--clients_count;
				sprintf(buf2, "server: client %d just left\n", clients[i].id);
				--i;
				broadcast(buf2, clients, clients_count, -1);
				continue;
			}
			buf[bytes] = '\0';
			sprintf(buf2, "client %d: %s", clients[i].id, buf);
			broadcast(buf2, clients, clients_count, i);
		}
	}
}
