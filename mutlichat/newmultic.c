#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 4445
#define BUF 1024

int main(int argc, char *argv[]) {
    int sock_fd;
    struct sockaddr_in server;
    char buffer[BUF];

    if (argc < 2) { printf("Usage: client <server_ip>\n"); exit(1); }

    // 1. Create socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Set server address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);  // IP from command line
    server.sin_port = htons(PORT);

    // 3. Connect to server
    connect(sock_fd, (struct sockaddr*)&server, sizeof(server));
    printf("Connected! Type messages:\n");

    // 4. Send and receive in a loop
    while (fgets(buffer, BUF, stdin) != NULL) {
        send(sock_fd, buffer, BUF, 0);          // send to server
        recv(sock_fd, buffer, BUF, 0);          // get echo back
        printf("Echo: %s\n", buffer);
    }

    return 0;
}
