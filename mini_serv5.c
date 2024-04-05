#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>

void error(char *msg)
{
	write(2, msg, strlen(msg));
	exit(1);
}

void fatal_error()
{
	error("Fatal error\n");
}

// whole message in buf
// if (1), new message in msg, buf updated
int extract_message(char **buf, char **msg)
{
	char	*newbuf;
	int	i;

	*msg = 0;
	if (*buf == 0)
		return (0);
	i = 0;
	while ((*buf)[i])
	{
		if ((*buf)[i] == '\n')
		{
			newbuf = calloc(1, sizeof(*newbuf) * (strlen(*buf + i + 1) + 1));
			if (newbuf == 0)
				return (-1);
			strcpy(newbuf, *buf + i + 1);
			*msg = *buf;
			(*msg)[i + 1] = 0;
			*buf = newbuf;
			return (1);
		}
		i++;
	}
	return (0);
}

char *str_join(char *buf, char *add)
{
	char	*newbuf;
	int		len;

	if (buf == 0)
		len = 0;
	else
		len = strlen(buf);
	newbuf = malloc(sizeof(*newbuf) * (len + strlen(add) + 1));
	if (newbuf == 0)
		return (0);
	newbuf[0] = 0;
	if (buf != 0)
		strcat(newbuf, buf);
	free(buf);
	strcat(newbuf, add);
	return (newbuf);
}

struct client {
	int fd;
	int id;
};


int main(int argc, char *argv[])
{
	if (argc < 2)
		fatal_error();
	int sockfd, connfd;
	struct sockaddr_in servaddr; 

	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1)
	bzero(&servaddr, sizeof(servaddr));
	// fcntl

	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(2130706433); //127.0.0.1
	servaddr.sin_port = htons(8081); 
  
	if ((bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr))) != 0 || listen(sockfd, 10) != 0)
		fatal_error();

	size_t clients_count = 0;
	struct client clients[1024] = {0};

	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(sockfd, &readfds);
	int maxfd = sockfd;
	int nextid = 0;

	while (1)
	{
		fd_set readfds_copy = readfds;
		if (select(connfd, &readfds_copy, 0, 0, 0) == -1)
			fatal_error();
		if (FD_ISSET(connfd, &readfds_copy))
	}

}