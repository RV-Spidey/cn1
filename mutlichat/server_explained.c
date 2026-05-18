/*
 * MULTI-CLIENT — SERVER SIDE
 *
 * This server handles multiple clients at the same time using fork().
 * Every time a new client connects, the server spawns a child process
 * to handle that client, while the parent goes back to waiting for more.
 *
 * How to run:  ./server
 * Then run multiple clients:  ./client 127.0.0.1
 */

#include <stdio.h>       /* printf */
#include <stdlib.h>      /* exit */
#include <string.h>      /* memset */
#include <sys/socket.h>  /* socket, bind, listen, accept, send, recv */
#include <netinet/in.h>  /* sockaddr_in, htons, INADDR_ANY */
#include <arpa/inet.h>   /* inet_ntop */
#include <unistd.h>      /* fork, close */

#define PORT 4444   /* port this server will listen on */
#define BUF  1024   /* max message size */

int main() {

    int server_fd;             /* listening socket — waits for new clients */
    int client_fd;             /* new socket created for each connected client */
    struct sockaddr_in server; /* server's own address (IP + port) */
    struct sockaddr_in client; /* will be filled with the connecting client's address */
    char buffer[BUF];          /* temporary storage for incoming/outgoing messages */
    int len;                   /* size of the client address struct */
    pid_t childpid;            /* stores the return value of fork() */

    /* ── STEP 1: CREATE A SOCKET ─────────────────────────────────────────
     * Same as the client side — creates an endpoint for communication.
     *   AF_INET     → IPv4
     *   SOCK_STREAM → TCP (reliable, connection-based)
     *   0           → OS picks TCP automatically */
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    printf("Socket created...\n");

    /* ── STEP 2: FILL IN THE SERVER'S OWN ADDRESS ───────────────────────
     *   sin_family      → AF_INET (IPv4)
     *   INADDR_ANY      → accept connections on ANY network interface
     *                     (localhost, LAN IP, etc. — don't restrict to one)
     *   htons(PORT)     → convert port to network byte order (big-endian) */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    /* ── STEP 3: BIND ────────────────────────────────────────────────────
     * bind() attaches the socket to the address we just filled in.
     * Think of it as: "reserve this port number for this program."
     * Without bind(), the OS doesn't know which port to listen on. */
    bind(server_fd, (struct sockaddr*)&server, sizeof(server));
    printf("Binding done...\n");

    /* ── STEP 4: LISTEN ──────────────────────────────────────────────────
     * listen() marks this socket as a passive socket — one that waits
     * for incoming connections (as opposed to actively connecting).
     * The second argument (5) is the backlog: how many pending connections
     * the OS will queue up before refusing new ones. */
    listen(server_fd, 5);
    printf("Waiting for connections...\n");

    /* ── STEP 5: ACCEPT LOOP — runs forever ──────────────────────────────
     * The server loops indefinitely, accepting one client at a time.
     * For each client, it forks a child to handle that client,
     * then immediately loops back to accept the next one. */
    while (1) {

        len = sizeof(client);

        /* accept() BLOCKS here — pauses the program until a client connects.
         * When a client connects, it returns a NEW socket (client_fd)
         * specifically for talking to that one client.
         * server_fd keeps listening; client_fd is the private line. */
        client_fd = accept(server_fd, (struct sockaddr*)&client, &len);
        printf("New client connected!\n");

        /* ── STEP 6: FORK — the key to handling multiple clients ─────────
         *
         * fork() creates an exact copy of the current process.
         * After fork(), TWO processes run from this point:
         *
         *   fork() returns 0      → you are the CHILD  process
         *   fork() returns > 0    → you are the PARENT process (gets child's PID)
         *
         *                   fork()
         *                  /      \
         *           CHILD (==0)   PARENT (>0)
         *           handles        closes client_fd
         *           this client    loops back to accept()
         *           forever        for the NEXT client
         */
        childpid = fork();

        if (childpid == 0) {
            /* ── CHILD PROCESS ───────────────────────────────────────────
             * The child's only job is to serve this one client.
             * It does NOT need the listening socket (server_fd),
             * so we close it to avoid resource leaks. */
            close(server_fd);

            /* Echo loop: receive a message → send it back, repeat. */
            while (1) {
                memset(buffer, 0, BUF); /* clear buffer before each recv */

                /* recv() BLOCKS until data arrives from the client.
                 * Stores the incoming message in buffer. */
                recv(client_fd, buffer, BUF, 0);
                printf("Received: %s\n", buffer);

                /* send() pushes the same buffer back to the client (echo). */
                send(client_fd, buffer, BUF, 0);
                printf("Echoed back: %s\n", buffer);
            }

        }

        /* ── PARENT PROCESS ──────────────────────────────────────────────
         * The parent does NOT handle this client — the child does.
         * So parent closes client_fd (it doesn't need it).
         * Then the while(1) loops back to accept() for the NEXT client. */
        close(client_fd);
    }

    return 0;
}
