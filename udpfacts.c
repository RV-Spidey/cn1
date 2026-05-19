#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <strings.h>
#include <unistd.h>

int factorial(int n)
{
    int a = 1;

    for(int i = 1; i <= n; i++)
    {
        a = a * i;
    }

    return a;
}

int main()
{
    int serversocket, port, n, a;

    struct sockaddr_in serveraddr, clientaddr;

    socklen_t len;

    serversocket = socket(AF_INET, SOCK_DGRAM, 0);

    bzero((char*)&serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;

    printf("Enter the port number: ");
    scanf("%d", &port);

    serveraddr.sin_port = htons(port);

    serveraddr.sin_addr.s_addr = INADDR_ANY;

    bind(serversocket,
         (struct sockaddr*)&serveraddr,
         sizeof(serveraddr));

    printf("\nWaiting for the client connection\n");

    bzero((char*)&clientaddr, sizeof(clientaddr));

    len = sizeof(clientaddr);

    recvfrom(serversocket,
             &n,
             sizeof(n),
             0,
             (struct sockaddr*)&clientaddr,
             &len);

    printf("\nNumber received from client.\n");

    a = factorial(n);

    printf("\nSending factorial to the client.\n");

    sendto(serversocket,
           &a,
           sizeof(a),
           0,
           (struct sockaddr*)&clientaddr,
           sizeof(clientaddr));

    close(serversocket);

    return 0;
}