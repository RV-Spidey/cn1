#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

int main()
{
    int serversocket, clientsocket, port;
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t len;
    char message[50];

    serversocket = socket(AF_INET, SOCK_STREAM, 0);

    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;

    printf("Enter the port: ");
    scanf("%d", &port);

    serveraddr.sin_port = htons(port);
    serveraddr.sin_addr.s_addr = INADDR_ANY;

    bind(serversocket, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

    bzero((char *)&clientaddr, sizeof(clientaddr));
    len = sizeof(clientaddr);

    listen(serversocket, 5);
    printf("Waiting for connection...\n");

    clientsocket = accept(serversocket, (struct sockaddr *)&clientaddr, &len);
    printf("Connected\n");

    read(clientsocket, message, sizeof(message));
    printf("Message received: %s\n", message);

    write(clientsocket, "hi", sizeof("hi"));

    close(clientsocket);
    close(serversocket);

    return 0;
}