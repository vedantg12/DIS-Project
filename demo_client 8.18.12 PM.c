#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

int main() {
    /* Initialize the sockets library */
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        exit(1);
    }
    
    /* Create a socket */
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        fprintf(stderr, "socket failed.\n");
        WSACleanup();
        exit(1);
    }
    
    /* Connect to the server */
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8000);
    serverAddr.sin_addr.s_addr = inet_addr("192.168.1.179");
    if (connect(clientSocket, (SOCKADDR *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        fprintf(stderr, "connect failed.\n");
        closesocket(clientSocket);
        WSACleanup();
        exit(1);
    }
    
    /* Send the request message to the server */
    int procedureID = 123;
    int arg1 = 456;
    char arg2[] = "hello";
    float result;
    if (send(clientSocket, (char *)&procedureID, sizeof(int), 0) != sizeof(int)) {
        fprintf(stderr, "send failed.\n");
        closesocket(clientSocket);
        WSACleanup();
        exit(1);
    }
    if (send(clientSocket, (char *)&arg1, sizeof(int), 0) != sizeof(int)) {
        fprintf(stderr, "send failed.\n");
        closesocket(clientSocket);
        WSACleanup();
        exit(1);
    }
    if (send(clientSocket, arg2, sizeof(arg2), 0) != sizeof(arg2)) {
        fprintf(stderr, "send failed.\n");
        closesocket(clientSocket);
        WSACleanup();
        exit(1);
    }
    
    /* Receive the response message from the server */
    if (recv(clientSocket, (char *)&result, sizeof(float), 0) != sizeof(float)) {
        fprintf(stderr, "recv failed.\n");
        closesocket(clientSocket);
        WSACleanup();
        exit(1);
    }
    
    /* Print the result */
    printf("Result = %f\n", result);
    
    /* Close the client socket and cleanup */
    closesocket(clientSocket);
    WSACleanup();
    
    return 0;
}
