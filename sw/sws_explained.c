/*
 * STOP AND WAIT ARQ — SERVER SIDE
 *
 * Receives frames from the client one at a time.
 * For each correct frame, sends an ACK back.
 * Client will not send the next frame until it gets the ACK.
 *
 * Uses UDP (SOCK_DGRAM) — reliability is added manually using ACKs.
 *
 * How to run:  ./sws
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>  /* socket, bind, recvfrom, sendto */
#include <arpa/inet.h>   /* htons, INADDR_ANY */

#define PORT 8082

/* Frame — the packet sent between client and server
 *   seq  → sequence number: which frame is this? (0, 1, 2...)
 *           server checks this to avoid accepting duplicate frames
 *   ack  → acknowledgement number: set by server in ACK reply
 *           server sets ack = expected + 1 meaning "got N, send N+1 next"
 *   type → 1 = data frame  (client → server)
 *           0 = ACK frame   (server → client)
 *   data → the actual message */
typedef struct {
    int seq, ack, type;
    char data[1024];
} Frame;

int main() {

    int sockfd;
    int expected = 0;              /* seq number we expect from client next */
    struct sockaddr_in server;     /* server's own address */
    struct sockaddr_in client;     /* filled by recvfrom() with sender's address
                                      needed so we know where to send the ACK back */
    socklen_t len = sizeof(client);
    Frame rf;                      /* rf = receive frame — incoming data from client */
    Frame af;                      /* af = ack frame    — ACK we send back to client */

    /* ── STEP 1: CREATE UDP SOCKET ──────────────────────────────────────────
     * SOCK_DGRAM = UDP. No connection, no handshake.
     * Stop-and-Wait manually adds reliability on top using seq numbers + ACKs. */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* ── STEP 2: FILL SERVER'S OWN ADDRESS ──────────────────────────────────
     *   sin_family      → AF_INET = IPv4
     *   sin_port        → htons converts port to network byte order (big-endian)
     *   INADDR_ANY      → accept packets from any network interface */
    server.sin_family      = AF_INET;
    server.sin_port        = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    /* ── STEP 3: BIND ────────────────────────────────────────────────────────
     * Reserves port 8082 for this program.
     * Without bind(), OS doesn't know which port belongs to this server. */
    bind(sockfd, (struct sockaddr *)&server, sizeof(server));
    printf("Server ready...\n");

    /* ── STEP 4: RECEIVE LOOP ────────────────────────────────────────────────*/
    while (1) {

        /* recvfrom() BLOCKS until a frame arrives from the client.
         * Also fills 'client' struct with sender's IP + port
         * so we can send the ACK back to the right place. */
        recvfrom(sockfd, &rf, sizeof(rf), 0, (struct sockaddr *)&client, &len);

        /* ── STEP 5: CHECK SEQUENCE NUMBER ───────────────────────────────────
         * rf.seq == expected → correct frame → accept and send ACK
         * rf.seq != expected → duplicate or wrong frame → ignore it
         *
         * Example: client sends frame 0 twice (retransmit).
         * First time:  expected=0, rf.seq=0 → match → accept
         * Second time: expected=1, rf.seq=0 → no match → ignore */
        if (rf.seq == expected) {
            printf("[+] Frame received: %s\n", rf.data);

            /* ── STEP 6: BUILD AND SEND ACK ──────────────────────────────────
             * af.type = 0          → ACK frame (not data)
             * af.ack  = expected+1 → "I got frame N, now send frame N+1"
             *
             * sendto() must be used with UDP — no connection exists,
             * so destination (client address) must be passed every time. */
            af.type = 0;
            af.seq  = 0;
            af.ack  = expected + 1;

            sendto(sockfd, &af, sizeof(af), 0, (struct sockaddr *)&client, len);
            printf("[+] ACK sent\n");

            expected++; /* move to next expected sequence number */
        } else {
            printf("[-] Wrong frame\n");
        }
    }

    return 0;
}
