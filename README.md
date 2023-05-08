# **DIS Project**

We have implemented four different load balancing algorithms and compared them. Here, we used three clients and three servers. These are the four algorithms:

1. Round Robin
2. Weighted Round Robin
3. Least Connection
4. Least Request Time

<br>

## **Setup**

First step will be common for every algorithm.

We need to create server's instances on three PCs.

<br>

Compile:`gcc server.c -o server.out`

<br>

Run three servers in three different PCs. Here, `<PORT i>` is the Port of that paricular server i on which we want to bind our socket.

Server PC 1 -> `./server.out <PORT1>`

Server PC 2 -> `./server.out <PORT2>`

Server PC 3 -> `./server.out <PORT3>`

<br>

Now, we need to compile the client file. So, three times we need to execute client file because three clients are there.

<br>

Compile: `gcc <client_file>.c -o <client_file>.out`

<br>

Run three clients in three different PCs.

Client PC 1 -> `./<client_file>.out <PORT1> <PORT2> <PORT3>`

Client PC 2 -> `./<client_file>.out <PORT1> <PORT2> <PORT3>`

Client PC 3 -> `./<client_file>.out <PORT1> <PORT2> <PORT3>`

<br>

Here, the `<client_file>` is different for each algorithm. These are the file names for each:

* Round Robin -> `client`

* Weighted Round Robin -> `client_wrr`

* Least Connection -> `client_lc`

* Least Request Time -> `client_lrt`

<br>

## **Execution**

A prompt will be seen where you need to give the file name without extension. The server will execute that file present on that server and send the output back to the client.

If you enter "exit", then the program will stop and show the current stats of three metrics.

<br>

## **Experiment**

We took 6 PCs where 3 were for server and 3 were for client. In those, three files `hello_1.c`, `hello_2.c` and `hello_3.c` with 0.1s, 0.2s and 0.4s of wait time in each of them respectively. In each, the "Hello World" is printed and the program waits for their respective given time.

<br>

We generated a test input comprising 1000 requests from each client to these servers. These requests were distributed by the load balancer. We calculated the stats for each of the four load balancing algorithms.

<br>

| **Algorithm** | Avg Response Time | Avg Throughput | Avg Server Utilization |
| :---        |    :----:   |          :---: | ---: |
| Round Robin | 682 ms | 958 req per sec | 0.83 |
| Weighted Round Robin | 543 ms | 1014 req per sec | 0.88 |
| Least Connection | 430 ms | 1014 req per sec | 0.91 |
| Least Response Time | 360 ms | 1268 req per sec | 0.89 |
