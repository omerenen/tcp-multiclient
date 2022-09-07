#include <stdio.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define PORT 6335

#define DATA_BUFFER 256
#define MAX_CONNECTIONS 3

int server_fd, new_fd, resp_val, i;
char buf[DATA_BUFFER];
fd_set read_fd_set;
struct sockaddr_in new_addr;
socklen_t addrlen;

int connection_list[MAX_CONNECTIONS];

int create_tcp_server_socket()
{
	struct sockaddr_in saddr;
	int fd, resp_val;

	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1)
	{
		fprintf(stderr, "socket failed [%s]\n", strerror(errno));
		return -1;
	}
	printf("Created a socket with fd: %d\n", fd);
	printf("-------------------------------\n");
	printf("Max connection size is : %d\n", MAX_CONNECTIONS-1);
	printf("-------------------------------\n");

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(PORT);
	saddr.sin_addr.s_addr = INADDR_ANY;

	resp_val = bind(fd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in));
	if (resp_val != 0)
	{
		fprintf(stderr, "bind failed [%s]\n", strerror(errno));
		close(fd);
		return -1;
	}

	resp_val = listen(fd, 5);
	if (resp_val != 0)
	{
		fprintf(stderr, "listen failed [%s]\n", strerror(errno));
		close(fd);
		return -1;
	}
	return fd;
}

void *th_send(void *arg)
{

	while (1)
	{
		bzero(buf, DATA_BUFFER);
		printf("Your message to *all* clients : \n");
		fgets(buf, DATA_BUFFER, stdin);
		for (i = 1; i < MAX_CONNECTIONS; i++)
		{
			int response = send(connection_list[i], buf, sizeof(buf), 0);
		}
	}
}
void *th_main(void *arg)
{
	while (1)
	{
		FD_ZERO(&read_fd_set);
		for (i = 0; i < MAX_CONNECTIONS; i++)
		{

			if (connection_list[i] >= 0)
			{
				FD_SET(connection_list[i], &read_fd_set);
			}
		}

		resp_val = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);
		
		if (resp_val >= 0)
		{
			if (FD_ISSET(server_fd, &read_fd_set))
			{
				new_fd = accept(server_fd, (struct sockaddr *)&new_addr, &addrlen);
				if (new_fd >= 0)
				{
					printf("Accepted a new connection with fd: %d\n", new_fd);
					for (i = 0; i < MAX_CONNECTIONS; i++)
					{
						if (connection_list[i] < 0)
						{
							connection_list[i] = new_fd;
							break;
						}
					}
				}
				else
				{
					fprintf(stderr, "accept failed [%s]\n", strerror(errno));
				}
				resp_val--;
				if (!resp_val)
					continue;
			}

			for (i = 1; i < MAX_CONNECTIONS; i++)
			{
				if ((connection_list[i] > 0) &&
					(FD_ISSET(connection_list[i], &read_fd_set)))
				{

					resp_val = recv(connection_list[i], buf, DATA_BUFFER, 0);

					if (resp_val == 0)
					{
						printf("Connection is closed for fd:%d\n", connection_list[i]);
						close(connection_list[i]);
						connection_list[i] = -1; 
					}
					if (resp_val > 0)
					{
						printf("Received data from client %d : %s\n",connection_list[i],
							buf);
					}
					if (resp_val == -1)
					{
						printf("Failed for fd: %d [%s]\n",
							   connection_list[i], strerror(errno));
						break;
					}
				}

				resp_val--;
				if (!resp_val)
					continue;
			}
		}
	}
}
int main()
{

	pthread_t thread_id[2];

	server_fd = create_tcp_server_socket();

	pthread_create(&thread_id[0], NULL, th_send, NULL);
	pthread_create(&thread_id[1], NULL, th_main, NULL);

	if (server_fd == -1)
	{
		fprintf(stderr, "Failed to create a server\n");
		return -1;
	}

	for (i = 0; i < MAX_CONNECTIONS; i++)
	{
		connection_list[i] = -1;
	}
	connection_list[0] = server_fd;

	pthread_join(thread_id[0], NULL);
	pthread_join(thread_id[1], NULL);

	return 0;
}