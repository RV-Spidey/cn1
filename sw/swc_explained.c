/*
 * STOP AND WAIT ARQ — CLIENT SIDE
 *
 * Sends frames to the server one at a time.
 * After each frame, STOPS and WAITS for an ACK.
 * If ACK correct  → send next frame (new data)
 * If ACK wrong    → resend same frame (retransmit)
 *
 * Uses UDP (SOCK_DGRAM) — reliability is added manually using ACKs.
 *
 * How to run:  ./swc
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>  /* socket, sendto, recvfrom */
#include <arpa/inet.h>   /* htons, inet_addr */

#define PORT 8082

/* Frame — the packet sent between client and server
 *   seq  → sequence number: which frame is this? (0, 1, 2...)
 *   ack  → acknowledgement number: set by server in ACK reply
 *           server sets ack = seq + 1 meaning "got N, send N+1 next"
 *   type → 1 = data frame  (client → server)
 *           0 = ACK frame   (server → client)
 *   data → the actual message */
typedef struct {
    int seq, ack, type;
    char data[1024];
} Frame;

int main() {

    int sockfd;
    int frame_id = 0;  /* sequence number of the frame we are currently sending */
    int need_new = 1;  /* 1 = ACK received last time → ask user for new data
                          0 = no ACK             → skip scanf, resend same frame */
    struct sockaddr_in server;     /* server's address — where to send frames */
    socklen_t len = sizeof(server);
    Frame sf;          /* sf = send frame — data frame we send to server */
    Frame rf;          /* rf = receive frame — ACK frame we get from server */

    /* ── STEP 1: CREATE UDP SOCKET ──────────────────────────────────────────
     * SOCK_DGRAM = UDP. Client does NOT call bind() or listen().
     * Client only needs to know the server's address to send to. */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* ── STEP 2: FILL SERVER'S ADDRESS ──────────────────────────────────────
     *   sin_family      → AF_INET = IPv4
     *   sin_port        → htons converts port to network byte order
     *   inet_addr()     → converts "127.0.0.1" string to binary IP number */
    server.sin_family      = AF_INET;
    server.sin_port        = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* ── STEP 3: SEND / WAIT LOOP ────────────────────────────────────────────*/
    while (1) {

        /* ── STEP 4: BUILD FRAME (only if last one was ACKed) ────────────────
         * need_new == 1 → previous frame delivered → build new frame with new data
         * need_new == 0 → no ACK came back         → sf still holds previous data
         *                                             so the same frame is resent */
        if (need_new) {
            sf.type = 1;        /* 1 = data frame */
            sf.seq  = frame_id; /* stamp with current sequence number */
            sf.ack  = 0;        /* ack unused in data frames */
            printf("Enter data: ");
            scanf("%s", sf.data);
        }

        /* ── STEP 5: SEND THE FRAME ──────────────────────────────────────────
         * sendto() — UDP version of send().
         * Destination must be specified every time (no persistent connection). */
        sendto(sockfd, &sf, sizeof(sf), 0, (struct sockaddr *)&server, sizeof(server));
        printf("[+] Frame sent\n");

        /* ── STEP 6: STOP — WAIT FOR ACK ─────────────────────────────────────
         * recvfrom() BLOCKS here — client does nothing until server replies.
         * This is the "STOP and WAIT" moment of the protocol. */
        recvfrom(sockfd, &rf, sizeof(rf), 0, (struct sockaddr *)&server, &len);

        /* ── STEP 7: CHECK IF ACK IS CORRECT ─────────────────────────────────
         * Server sets rf.ack = frame_id + 1 when it receives our frame.
         *
         * rf.ack == frame_id + 1
         *   TRUE  → server got the frame → advance to next frame
         *   FALSE → something went wrong → resend same frame */
        if (rf.ack == frame_id + 1) {
            printf("[+] ACK received\n");
            frame_id++;    /* advance sequence number */
            need_new = 1;  /* get new data from user next iteration */
        } else {
            printf("[-] ACK not received — resending\n");
            need_new = 0;  /* skip scanf, resend same frame next iteration */
        }
    }

    return 0;
}
