#include<stdio.h> //printf
#include <stdlib.h>
#include<string.h>    //strlen
#include <unistd.h>
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include <netinet/in.h>
 
int main(int argc , char *argv[])
{
	int sock;
	struct sockaddr_in server;
	char message[1000] , server_reply[2000];

	//Create socket
	sock = socket(AF_INET , SOCK_STREAM , 0);
	if (sock == -1)
    {
		printf("Could not create socket");
    }
	puts("Socket created");

	server.sin_addr.s_addr = inet_addr("192.168.1.70"); //100.35.37.229
	server.sin_family = AF_INET;
	server.sin_port = htons(1234);
 
    	//Connect to remote server

	if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		perror("connect failed. Error");
		return 1;
	}
	puts("Connected\n");

	//keep communicating with server
	while(1)
	{
		printf("Enter message : ");
		gets(message);

		char m[2000];

		int i;

		strcpy(m,message);

		//Send some data
		if( send(sock , m , strlen(m) , 0) < 0)
		{
			puts("Send failed");
			return 1;
		}

		while(1)
		{        
			//Receive a reply from the server
			if( recv(sock , server_reply , 2000 , 0) < 0)
			{
				puts("recv failed");
				break;
			}
			puts(server_reply);
			if(strcmp(server_reply,"!@#$%^&*()")>0)
				break;
			memset(server_reply,0,strlen(server_reply));
		}
	}

	close(sock);
	return 0;
}
