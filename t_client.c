#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <pthread.h>
#include <errno.h>
#include <arpa/inet.h>

extern int errno;
int errnum;


#define PORT 6335
#define SA struct sockaddr
int sockfd, connfd;
int buffer_size = 256;

void *th_listen(void *arg)
{
	char buff[buffer_size];
	while (1)
	{
		bzero(buff, buffer_size);
		int response = recv(sockfd, buff, buffer_size, 0);

		if (response == -1)
		{
			errnum = errno;

			fprintf(stderr, "Error : %s\n", strerror(errnum));
		}
		else
		{
			printf("Received message : %s\n", buff);
		}
	
	}

}

void *th_send(void *arg)
{

	char buff[buffer_size];
	while (1)
	{
		bzero(buff, buffer_size);
		printf("Your message :\t");
		fgets(buff, buffer_size, stdin);
		int response = send(sockfd, buff, sizeof(buff), 0);
	}
}

int main()
{

	pthread_t thread_id[2];

	struct sockaddr_in servaddr, cli;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("socket creation failed...\n");
		exit(0);
	}
	else
	{
		printf("Socket successfully created..\n");
	}

	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);

	if (connfd = connect(sockfd, (SA *)&servaddr, sizeof(servaddr)) != 0)
	{
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
	{
		printf("connected to the server..\n");
	}

	
	pthread_create(&thread_id[0], NULL, th_listen, NULL);
	pthread_create(&thread_id[1], NULL, th_send, NULL);

	pthread_join(thread_id[0], NULL);
	pthread_join(thread_id[1], NULL);
	close(sockfd);
}