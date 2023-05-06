#include <stdio.h>      //printf
#include <string.h>     //strlen
#include <sys/socket.h> //socket
#include <arpa/inet.h>  //inet_addr
#include <unistd.h>
#include <limits.h>
#include <time.h>

struct queue
{
	char string[2000];
	struct queue *next;
    time_t start_time;
};

struct thread_args
{
    int socket;
    int idx;
};

static int file_number = 0;
static int curr_file_number = 0;

double server_avg_resp_time[3] = {0, 0, 0};
int query_processed[3] = {0, 0, 0};

struct queue* heads[3];
struct queue* tails[3];

static int server_id = -1;

int load_balancer()
{

    int minidx, minAvgRespTime = INT_MAX;

    for(int i = 0; i < 3; i++)
    {
        if(heads[i] == NULL)
        {
            return i;
        }
        else if(server_avg_resp_time[i] < minAvgRespTime)
        {
            minAvgRespTime = tails[i]->start_time;
            minidx = i;
        }
    }

    return minidx;
}

void enqueue(char *re, int i)
{
    time_t start_time;
    time(&start_time);
	if (heads[i] == NULL)
	{
		heads[i] = (struct queue *)malloc(sizeof(struct queue));
		strcpy(heads[i]->string, re);
		tails[i] = heads[i];
		heads[i]->next = NULL;
	}
	else
	{
		tails[i]->next = (struct queue *)malloc(sizeof(struct queue));
		tails[i] = tails[i]->next;
		strcpy(tails[i]->string, re);
		tails[i]->next = NULL;
	}
    tails[i]->start_time = start_time;
}

struct queue *dequeue(int i)
{
	struct queue *temp;
	temp = heads[i];
	heads[i] = heads[i]->next;
	return temp;
}

void *communicate_with_server(void *ta)
{
	char buffer[2000];
	struct thread_args targ = *(struct thread_args *)ta;
    int socket = targ.socket;
    int idx = targ.idx;
	while(1)
	{
        if(heads[idx] != NULL)
        {
            printf("%s\n", heads[idx]->string);
            if (send(socket, heads[idx]->string, strlen(heads[idx]->string), 0) < 0)
            {
                puts("Send failed");
                return;
            }
            printf("%s\n", heads[idx]->string);
            time_t end_time;
            while(1)
            {
                if(recv(socket, buffer, 2000, 0) > 0)
                {
                    printf("%s\n", buffer);
                    if(strcmp(buffer, "!@#$%^&*()") == 0)
                    {
                        time(&end_time);
                        query_processed[idx] += 1;
                        memset(buffer, 0, strlen(buffer));
                        break;
                    }
                    memset(buffer, 0, strlen(buffer));
                }
            }
            double diff_t = difftime(end_time,tails[idx]->start_time);
            server_avg_resp_time[idx] += diff_t;
            server_avg_resp_time[idx] = (double)server_avg_resp_time[idx]/query_processed[idx];
            dequeue(idx);
        }
	}
}

int main(int argc, char *argv[])
{
    for(int i=0; i<3; i++)
    {
        heads[i] = NULL;
        tails[i] = NULL;
    }
    int sockets[3];
    struct sockaddr_in servers[3];
    char message[1000];

    // Create socket
    sockets[0] = socket(AF_INET, SOCK_STREAM, 0);
    if (sockets[0] == -1)
    {
        printf("Could not create socket 1");
    }
    puts("Socket 1 created");

    sockets[1] = socket(AF_INET, SOCK_STREAM, 0);
    if (sockets[1] == -1)
    {
        printf("Could not create socket 2");
    }
    puts("Socket created 2");

    sockets[2] = socket(AF_INET, SOCK_STREAM, 0);
    if (sockets[2] == -1)
    {
        printf("Could not create socket 3");
    }
    puts("Socket 3 created");

    servers[0].sin_addr.s_addr = inet_addr("127.0.0.1");
    servers[0].sin_family = AF_INET;
    servers[0].sin_port = htons(atoi(argv[1]));

    servers[1].sin_addr.s_addr = inet_addr("127.0.0.1");
    servers[1].sin_family = AF_INET;
    servers[1].sin_port = htons(atoi(argv[2]));

    servers[2].sin_addr.s_addr = inet_addr("127.0.0.1");
    servers[2].sin_family = AF_INET;
    servers[2].sin_port = htons(atoi(argv[3]));

    if (connect(sockets[0], (struct sockaddr *)&servers[0], sizeof(servers[0])) < 0)
    {
        perror("connect failed. Error in connecting to server 1");
        return 1;
    }

    puts("Connected successfully to server 1\n");

    if (connect(sockets[1], (struct sockaddr *)&servers[1], sizeof(servers[1])) < 0)
    {
        perror("connect failed. Error in connecting to server 2");
        return 1;
    }
    
    puts("Connected successfully to server 2\n");

    if (connect(sockets[2], (struct sockaddr *)&servers[2], sizeof(servers[2])) < 0)
    {
        perror("connect failed. Error in connecting to server 3");
        return 1;
    }

    puts("Connected successfully to server 3\n");

    pthread_t read_t[3];
    int rc;

    struct thread_args ta[3];

    ta[0].socket = sockets[0];
    ta[0].idx = 0;
    ta[1].socket = sockets[1];
    ta[1].idx = 1;
    ta[2].socket = sockets[2];
    ta[2].idx = 2;

    if (pthread_create(&read_t[0], NULL, communicate_with_server, (void *)&ta[0]) < 0)
    {
        perror("could not create thread for server 1");
        return 1;
    }
    else
    {
        printf("Thread created in Client for server 1\n");
    }

    if (pthread_create(&read_t[1], NULL, communicate_with_server, (void *)&ta[1]) < 0)
    {
        perror("could not create thread for server 2");
        return 1;
    }
    else
    {
        printf("Thread created in Client for server 2\n");
    }

    if (pthread_create(&read_t[2], NULL, communicate_with_server, (void *)&ta[2]) < 0)
    {
        perror("could not create thread for server 3");
        return 1;
    }
    else
    {
        printf("Thread created in Client for server 3\n");
    }

    // if (pthread_create(&write_t[0], NULL, write_message, (void *)&ta[0]) < 0)
    // {
    //     perror("could not create thread for server 1");
    //     return 1;
    // }
    // else
    // {
    //     printf("Thread created in Client for server 1\n");
    // }

    // if (pthread_create(&write_t[1], NULL, write_message, (void *)&ta[1]) < 0)
    // {
    //     perror("could not create thread for server 2");
    //     return 1;
    // }
    // else
    // {
    //     printf("Thread created in Client for server 2\n");
    // }

    // if (pthread_create(&write_t[2], NULL, write_message, (void *)&ta[2]) < 0)
    // {
    //     perror("could not create thread for server 3");
    //     return 1;
    // }
    // else
    // {
    //     printf("Thread created in Client for server 3\n");
    // }

    // keep communicating with server
    while (1)
    {
        printf("Enter message :");
        scanf("%s", message);

        int idx = load_balancer();
        enqueue(message, idx);
    }

    rc = pthread_join(read_t[0], NULL);
    rc = pthread_join(read_t[1], NULL);
    rc = pthread_join(read_t[2], NULL);

    printf("I am before close!");
    close(sockets[0]);
    return 0;
}
