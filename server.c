// socket server example, handles multiple clients using threads

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h> //strlen
#include <sys/socket.h>
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <pthread.h>   //for threading , link with lpthread

// the thread function
void *connection_handler(void *);

int main(int argc, char *argv[])
{
	int socket_desc, client_sock, c, *new_sock;
	struct sockaddr_in server, client;

	// Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(atoi(argv[1]));

	// Bind
	if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		// print the error message
		perror("bind failed. Error");
		return 1;
	}
	printf("Bind done\n");

	// Listen
	listen(socket_desc, 3);

	// Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);

	c = sizeof(struct sockaddr_in);
	client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);
	if(client_sock < 0)
	{
		perror("accept");
        exit(EXIT_FAILURE);
	}
	puts("Connection accepted");

	pthread_t read_t;
	int rc;
	new_sock = malloc(1);
	*new_sock = client_sock;

	if (pthread_create(&read_t, NULL, connection_handler, (void *)new_sock) < 0)
	{
		perror("could not create thread");
		return 1;
	}
	else
	{
		printf("Thread created\n");
	}

	rc = pthread_join(read_t, NULL);
	printf("%d\n", rc);

	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}

	close(client_sock);

	return 0;
}

/*
	This will handle connection for each client
	*/
void *connection_handler(void *socket_desc)
{
	while(1)
	{
		// Get the socket descriptor
		int sock = *(int *)socket_desc;
		int n;

		char sendBuff[100], client_message[2000];

		while ((n = recv(sock, client_message, 2000, 0)) > 0)
		{
			char command[2000];
			sprintf(command, "gcc %s.c -o %s.out;./%s.out> temp", client_message, client_message, client_message);
			printf("%s\n", command);
			printf("%s\n", client_message);
			system(command);
			FILE *fp;
			char *line = NULL;
			size_t len = 0;
			ssize_t read;

			fp = fopen("temp", "r");
			if (fp == NULL)
				exit(EXIT_FAILURE);

			while ((read = getline(&line, &len, fp)) != -1)
			{
				// printf("Retrieved line of length %zu :\n", read);
				printf("%s", line);
				write(sock, line, strlen(line));
			}
			write(sock, "!@#$%^&*()", 10);
			fclose(fp);
			if (line)
				free(line);
		}
	}
	return 0;
}
