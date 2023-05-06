#include <stdio.h>      //printf
#include <string.h>     //strlen
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <unistd.h>     //write
#include <stdlib.h>     //strlen

// the thread function
void *connection_handler(void *);

int main(int argc, char *argv[])
{
    // socket to server
    int sock_server;
    struct sockaddr_in server;

    // Create socket to server
    sock_server = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_server == -1)
    {
        printf("Could not create socket to server");
    }

    puts("Socket to server created");

    // map the ip of server and socket
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(3000);

    // Connect to remote server
    if (connect(sock_server, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect failed. Error! could not connect to server");
        return 1;
    }

    puts("Connected to server\n");

    // bind socket to client
    int socket_desc, client_sock, c, *new_sock;
    struct sockaddr_in middleware, client;

    middleware.sin_family = AF_INET;
    middleware.sin_addr.s_addr = INADDR_ANY;
    middleware.sin_port = htons(2999);

    // Bind
    if (bind(socket_desc, (struct sockaddr *)&middleware, sizeof(middleware)) < 0)
    {
        // print the error message
        perror("bind failed. Error in binding in middleware!");
        return 1;
    }
    puts("bind done in middleware!");

    // Listen
    listen(socket_desc, 3);

    // Accept and incoming connection
    puts("Waiting for incoming requests from client...");
    c = sizeof(struct sockaddr_in);
    while (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c))
    {
        puts("Connection accepted");
        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = client_sock;
        if (pthread_create(&sniffer_thread, NULL, connection_handler, (void *)new_sock) < 0)
        {
            perror("could not create thread");
            return 1;
        }
        puts("Handler assigned");
    }
    if (client_sock < 0)
    {
        perror("accept failed");
        return 1;
    }
    return 0;
}
/*
	This will handle connection for each client
*/
void *connection_handler(void *socket_desc)
{
    int sock = *(int *)socket_desc;
    int n;
    char sendBuff[100], client_message[2000];
    while ((n = recv(sock, client_message, 2000, 0)) > 0)
    {
        
    }
}