#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>

int main()
{
    int clientsocket, port, n, fact;

    struct sockaddr_in serveraddr;

    socklen_t len;

    clientsocket = socket(AF_INET, SOCK_DGRAM, 0);

    bzero((char*)&serveraddr, sizeof(serveraddr));

    len = sizeof(serveraddr);

    serveraddr.sin_family = AF_INET;

    printf("Enter the port number: ");
    scanf("%d", &port);

    serveraddr.sin_port = htons(port);

    serveraddr.sin_addr.s_addr = INADDR_ANY;

    printf("\nSending message for server connection\n");

    printf("\nEnter the number:\n");
    scanf("%d", &n);

    sendto(clientsocket,
           &n,
           sizeof(n),
           0,
           (struct sockaddr*)&serveraddr,
           sizeof(serveraddr));

    printf("\nReceiving factorial from server.\n");

    recvfrom(clientsocket,
             &fact,
             sizeof(fact),
             0,
             (struct sockaddr*)&serveraddr,
             &len);

    printf("\nFactorial is:\t%d\n", fact);

    close(clientsocket);

    return 0;
}