#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

int main()
{
    int clientsocket, port;
    struct sockaddr_in serveraddr;
    socklen_t len;
    char message[50];

    clientsocket = socket(AF_INET, SOCK_STREAM, 0);

    bzero((char *)&serveraddr, sizeof(serveraddr));
    len = sizeof(serveraddr);
    serveraddr.sin_family = AF_INET;

    printf("Enter the port: ");
    scanf("%d", &port);

    serveraddr.sin_port = htons(port);

    printf("trying to connect\n");
    connect(clientsocket,(struct sockaddr*)&serveraddr,len);
    printf("Connected to the server\n");

    send(clientsocket, "client", sizeof("client"),0);
    printf("Message sent");


    recv(clientsocket, message, sizeof(message),0);
    printf("Message received: %s\n", message);
    close(clientsocket);

}