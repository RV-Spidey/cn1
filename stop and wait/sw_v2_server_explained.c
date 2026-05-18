/*
 * STOP AND WAIT ARQ — SERVER SIDE
 *
 * Receives frames from the client one at a time.
 * For each frame received correctly, sends an ACK back.
 * The client will not send the next frame until it gets the ACK.
 *
 * Uses UDP — no connection needed, just send/receive packets.
 *
 * How to run:  ./sw_v2_server
 */

#include <stdio.h>       /* printf */
#include <stdlib.h>      /* exit */
#include <string.h>      /* memset */
#include <sys/socket.h>  /* socket, bind, recvfrom, sendto */
#include <arpa/inet.h>   /* htons, INADDR_ANY */

#define PORT 8080

/* Frame is the envelope sent between client and server.
 *   seq  → sequence number: which frame is this? (0, 1, 2, 3...)
 *           prevents duplicate frames from being accepted twice
 *   ack  → acknowledgement number: used only in ACK replies
 *           server sets ack = seq + 1 meaning "got frame N, send N+1 next"
 *   type → 1 = data frame (client sending data)
 *           0 = ACK  frame (server confirming receipt)
 *   data → the actual message text */
typedef struct {
    int  seq;
    int  ack;
    int  type;
    char data[1024];
} Frame;

int main() {

    int sockfd;                    /* our UDP socket */
    int expected = 0;              /* seq number we expect from client next
                                      starts at 0, goes up by 1 each time */
    struct sockaddr_in server;     /* server's own address (our address) */
    struct sockaddr_in client;     /* filled automatically by recvfrom()
                                      tells us WHO sent the frame so we can
                                      send the ACK back to the right place */
    socklen_t len = sizeof(client);
    Frame recv_frame;              /* frame received FROM client */
    Frame ack_frame;               /* ACK frame we send BACK to client */

    /* ── STEP 1: CREATE A UDP SOCKET ────────────────────────────────────────
     * SOCK_DGRAM = UDP (connectionless — no handshake, no persistent connection)
     * Unlike TCP, UDP just fires packets. Stop-and-Wait adds reliability manually
     * on top using sequence numbers and ACKs. */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* ── STEP 2: FILL IN THE SERVER'S OWN ADDRESS ───────────────────────────
     *   sin_family   → AF_INET = IPv4
     *   sin_port     → htons(PORT) converts port to network byte order (big-endian)
     *   INADDR_ANY   → accept packets arriving on ANY network interface
     *                  (localhost, LAN, etc.) — don't restrict to one IP */
    memset(&server, 0, sizeof(server));
    server.sin_family      = AF_INET;
    server.sin_port        = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    /* ── STEP 3: BIND ────────────────────────────────────────────────────────
     * Attaches our socket to the address we filled above.
     * Without bind(), the OS doesn't know which port belongs to this program.
     * Clients do NOT need bind(). Only servers do. */
    bind(sockfd, (struct sockaddr *)&server, sizeof(server));
    printf("Server ready on port %d...\n", PORT);

    /* ── STEP 4: RECEIVE LOOP ────────────────────────────────────────────────
     * Loop forever — keep accepting frames from the client. */
    while (1) {

        /* recvfrom() — UDP version of recv()
         * BLOCKS here until a frame arrives from anywhere.
         * Unlike recv(), it also fills the 'client' struct with the sender's
         * IP and port — we NEED this to know where to send the ACK back.
         *
         * Arguments:
         *   sockfd       → our socket
         *   &recv_frame  → where to store the incoming frame
         *   sizeof(Frame)→ max bytes to read
         *   0            → no flags
         *   &client      → OUTPUT: sender's address is written here
         *   &len         → size of the client struct */
        recvfrom(sockfd, &recv_frame, sizeof(Frame), 0,
                 (struct sockaddr *)&client, &len);

        /* ── STEP 5: CHECK IF FRAME IS THE ONE WE EXPECTED ──────────────────
         * recv_frame.seq == expected checks the sequence number.
         *   Match    → correct frame, accept it and send ACK
         *   No match → duplicate or out-of-order frame, ignore it
         *
         * Example: if we already received frame 0 and client resends it,
         * recv_frame.seq=0 but expected=1 → mismatch → we ignore it. */
        if (recv_frame.seq == expected) {

            printf("[+] Frame %d received: %s\n", recv_frame.seq, recv_frame.data);

            /* ── STEP 6: BUILD AND SEND ACK ──────────────────────────────────
             * ack_frame.type = 0          → this is an ACK frame (not data)
             * ack_frame.ack  = expected+1 → "I got frame N, send frame N+1 next"
             *
             * sendto() — UDP version of send()
             * Must pass destination (client) every time since UDP has no connection. */
            ack_frame.type = 0;
            ack_frame.seq  = 0;
            ack_frame.ack  = expected + 1;

            sendto(sockfd, &ack_frame, sizeof(ack_frame), 0,
                   (struct sockaddr *)&client, len);

            printf("[+] ACK %d sent\n", ack_frame.ack);

            expected++; /* move to next expected sequence number */
        }
        else {
            /* Wrong sequence number — could be a duplicate or corrupted frame.
             * We simply ignore it. Client will timeout and resend. */
            printf("[-] Wrong frame (got %d, expected %d) — ignored\n",
                   recv_frame.seq, expected);
        }
    }

    return 0;
}
