#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 4445
#define BUF 1024

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server, client;
    char buffer[BUF];
    int len;
    pid_t childpid;

    // 1. Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Set up address
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;  // accept from any IP
    server.sin_port = htons(PORT);

    // 3. Bind socket to port
    bind(server_fd, (struct sockaddr*)&server, sizeof(server));

    // 4. Listen for connections
    listen(server_fd, 5);
    printf("Server listening on port %d...\n", PORT);

    while (1) {
        len = sizeof(client);

        // 5. Accept a new client
        client_fd = accept(server_fd, (struct sockaddr*)&client, &len);
        printf("New client connected!\n");

        // 6. Fork: create child process to handle THIS client
        childpid=fork();
        if (childpid == 0) {
            // --- CHILD PROCESS ---
            close(server_fd);  // child doesn't need the listening socket

            while (1) {
                memset(buffer, 0, BUF);
                recv(client_fd, buffer, BUF, 0);   // receive message
                printf("Got: %s\n", buffer);
                send(client_fd, buffer, BUF, 0);   // echo it back
            }
        }

        // --- PARENT PROCESS ---
        close(client_fd);  // parent doesn't need this; child handles it
    }
}
