#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

int main() {
    int clientsocket, port;
    struct sockaddr_in serveraddr;
    socklen_t len;
    char message[50];

    clientsocket = socket(AF_INET, SOCK_STREAM, 0);
    bzero((char*)&serveraddr, sizeof(serveraddr));
    len = sizeof(serveraddr);
    serveraddr.sin_family = AF_INET;

    printf("Enter the port number:");
    scanf("%d", &port);

    serveraddr.sin_port = htons(port);

    printf("\n Trying to connect to the server.\n");

    connect(clientsocket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));

    printf("\n Connected to the server.\n");

    printf("\n Sending message for server connection.\n");

    send(clientsocket, "HI, I AM CLIENT...", sizeof("HI, I AM CLIENT..."), 0);

    printf("\n Receiving message from server. \n");

    recv(clientsocket, message, sizeof(message), 0);

    printf("\n message recieved\t%s\n", message);

    close(clientsocket);

    return 0;
}
