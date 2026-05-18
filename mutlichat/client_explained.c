/*
 * MULTI-CLIENT — CLIENT SIDE
 *
 * This program connects to the multi-client server and sends messages.
 * The server echoes every message back, and we print it.
 *
 * How to run:  ./client <server_ip>
 * Example:     ./client 127.0.0.1
 */

#include <stdio.h>       /* printf, fgets */
#include <stdlib.h>      /* exit */
#include <string.h>      /* memset, strlen */
#include <sys/socket.h>  /* socket, connect, send, recv */
#include <netinet/in.h>  /* sockaddr_in, htons */
#include <arpa/inet.h>   /* inet_addr */

#define PORT 4444   /* must match the port the server is listening on */
#define BUF  1024   /* max size of one message */

int main(int argc, char *argv[]) {

    int sock_fd;               /* our socket — the "phone" we'll talk through */
    struct sockaddr_in server; /* holds the server's IP address and port */
    char buffer[BUF];          /* temporary storage for messages we type/receive */

    /* argc is the argument count. argc < 2 means the user forgot to pass an IP.
     * argv[0] = program name, argv[1] = server IP (e.g. "127.0.0.1") */
    if (argc < 2) {
        printf("Usage: client <server_ip>\n");
        exit(1);
    }

    /* ── STEP 1: CREATE A SOCKET ─────────────────────────────────────────
     * Think of socket() as picking up a telephone handset.
     *   AF_INET     → use IPv4 addresses
     *   SOCK_STREAM → use TCP (reliable, ordered, connection-based)
     *   0           → kernel picks the right protocol automatically (TCP here)
     * Returns a file descriptor (an integer) used for all future calls. */
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    /* ── STEP 2: FILL IN THE SERVER'S ADDRESS ────────────────────────────
     * sockaddr_in is a struct that holds "who to call":
     *   sin_family → address family, always AF_INET for IPv4
     *   sin_addr   → server's IP converted from string "127.0.0.1" to binary
     *   sin_port   → port number in network byte order
     *
     * htons() = "Host TO Network Short"
     *   Converts port from your CPU's byte order to network byte order (big-endian).
     *   Always required when setting port numbers.
     *
     * inet_addr() converts dotted-decimal string "127.0.0.1"
     *   into the 32-bit binary number the struct needs. */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(PORT);

    /* ── STEP 3: CONNECT TO THE SERVER ───────────────────────────────────
     * connect() is like dialing the number.
     * The OS performs the TCP three-way handshake (SYN → SYN-ACK → ACK).
     * We cast to (struct sockaddr*) because connect() is generic — it works
     * for any address family, not just IPv4, so it expects a generic pointer. */
    connect(sock_fd, (struct sockaddr*)&server, sizeof(server));
    printf("Connected! Type messages:\n");

    /* ── STEP 4: SEND AND RECEIVE IN A LOOP ──────────────────────────────
     * fgets() reads one line from the keyboard (stdin) into buffer.
     * Returns NULL when the user presses Ctrl+D (EOF), ending the loop.
     *
     * Each loop iteration:
     *   1. User types a message  → fgets() stores it in buffer
     *   2. send() pushes the buffer to the server over the socket
     *   3. recv() blocks (waits) for the server's reply, stores it in buffer
     *   4. We print what came back
     *
     * send() / recv() arguments:
     *   sock_fd → which socket to use
     *   buffer  → where the data is / should land
     *   BUF     → max bytes to send/receive
     *   0       → no special flags */
    while (fgets(buffer, BUF, stdin) != NULL) {
        send(sock_fd, buffer, BUF, 0); /* send typed message to server */
        recv(sock_fd, buffer, BUF, 0); /* wait for server's echo reply */
        printf("Echo: %s\n", buffer);
    }

    /* When loop ends (Ctrl+D), the OS closes sock_fd automatically on exit. */
    return 0;
}
