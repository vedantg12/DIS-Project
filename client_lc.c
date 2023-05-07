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
    int length;
    time_t start_time;
};

struct thread_args
{
    int socket;
    int idx;
};

static int file_number = 0;
static int curr_file_number = 0;

struct queue* heads[3];
struct queue* tails[3];

double server_avg_resp_time[3] = {0, 0, 0};
int query_processed[3] = {0, 0, 0};
int server_avg_server_utilization_time[3] = {0, 0, 0};

static int server_id = -1;

int load_balancer()
{
    // if(heads[0] == NULL && heads[1] == NULL && heads[2] == NULL){
    //     return 0;
    // }
    // else
    // {
    //     int min_len_idx = 0;
    //     int min_len = INT_MAX;
    //     for(int i=0; i<3; i++)
    //     {
    //         if(heads[i]->length < min_len){
    //             min_len = heads[i]->length;
    //             min_len_idx = i;
    //         }
    //     }
    //     return min_len_idx;
    // }

    int minidx, minlen = INT_MAX;

    for(int i = 0; i < 3; i++)
    {
        if(heads[i] == NULL)
        {
            return i;
        }
        else if(heads[i]->length < minlen)
        {
            minlen = heads[i]->length;
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
        heads[i]->length = 1;
	}
	else
	{
		tails[i]->next = (struct queue *)malloc(sizeof(struct queue));
		tails[i] = tails[i]->next;
		strcpy(tails[i]->string, re);
		tails[i]->next = NULL;
        heads[i]->length++;
	}
    tails[i]->start_time = start_time;
}

struct queue *dequeue(int i)
{
	struct queue *temp;
	temp = heads[i];
    heads[i]->length--;
	heads[i] = heads[i]->next;
	return temp;
}

void *communicate_with_server(void *ta)
{
	char buffer[2000];
	struct thread_args targ = *(struct thread_args *)ta;
    int socket = targ.socket;
    int idx = targ.idx;
    int flag = 0;
	while(1)
	{
        int break_flag = 0;
        if(heads[idx] != NULL)
        {
            if (send(socket, heads[idx]->string, strlen(heads[idx]->string), 0) < 0)
            {
                puts("Send failed");
                return;
            }
            time_t end_time;
            while(!break_flag)
            {
                if(recv(socket, buffer, 2000, 0) > 0)
                {
                    if(strchr(buffer, '#') != NULL)
                    {
                        char* token = strtok(buffer, "#");
                        while(token != NULL)
                        {
                            if(flag)
                            {
                                char* p;
                                int server_time = atoi(token);
                                printf("Inside flag condition\n");
                                printf("The value is %d\n", server_time);
                                printf("Token is %s\n", token);
                                server_avg_server_utilization_time[idx] += server_time;
                                flag = 0;
                            }
                            else if(strcmp(token, "serverUtilizationTime") == 0)
                            {
                                flag = 1;
                            }
                            else if(strcmp(token, "!@$%^&*()") == 0)
                            {
                                time(&end_time);
                                query_processed[idx] += 1;
                                memset(buffer, 0, strlen(buffer));
                                break_flag = 1;
                                break;
                            }
                            token = strtok(NULL, "#");
                        }
                    }
                    memset(buffer, 0, strlen(buffer));
                }
            }
            double diff_t = difftime(end_time, heads[idx]->start_time);
            printf("Time taken: %f\n", diff_t);
            double server_time = server_avg_resp_time[idx] * (query_processed[idx] - 1);
            server_avg_resp_time[idx] = (server_time + diff_t)/query_processed[idx];
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

    // keep communicating with server
    while (1)
    {
        printf("Enter message or type exit to exit the program:");
        scanf("%s", message);

        if(strcmp(message, "exit") == 0)
        {
            printf("Printing out the Load Balancer Stats:\n");
            printf("Server average Response Time:\n");
            double avg_resp_time = (server_avg_resp_time[0] + server_avg_resp_time[1] + server_avg_resp_time[2])/3.0;
            printf("%f\n", server_avg_resp_time[0]);
            printf("Server average Throughput:\n");
            double avg_throughput = ((query_processed[0]+0.001)/server_avg_resp_time[0]) + (query_processed[1]+0.001/server_avg_resp_time[1]) + (query_processed[2]+0.001/server_avg_resp_time[2])/3.0;
            printf("%f\n", avg_throughput);
            printf("Server average Utilization:\n");
            double avg_server_utilization = ((server_avg_server_utilization_time[0])/server_avg_resp_time[0] + (server_avg_server_utilization_time[1])/server_avg_resp_time[1] + (server_avg_server_utilization_time[2])/server_avg_resp_time[2])/3.0;
            printf("%f\n", avg_server_utilization);
            break;
        }

        int idx = load_balancer();
        enqueue(message, idx);

    }

    rc = pthread_join(read_t[0], NULL);
    rc = pthread_join(read_t[1], NULL);
    rc = pthread_join(read_t[2], NULL);
    
    close(sockets[0]);
    return 0;
}
