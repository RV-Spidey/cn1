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

    clientsocket = socket(AF_INET, SOCK_DGRAM, 0);

    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;

    printf("Enter the port: ");
    scanf("%d", &port);

    serveraddr.sin_port = htons(port);
    len = sizeof(serveraddr);

    printf("waitimg for the connection\n");
    sendto(clientsocket, "hi",sizeof("hi"), 0, (struct sockaddr *)&serveraddr, sizeof(serveraddr));
    ///printf("Message received: %s\n", message);
    recvfrom(clientsocket, message, sizeof(message), 0, (struct sockaddr *)&serveraddr,&len);
    printf("Message received: %s\n", message);
    close(clientsocket);

    return 0;
}