/*
 * STOP AND WAIT ARQ — CLIENT (EXPLAINED)
 *
 * Stop-and-Wait rule for the client:
 *   1. Send ONE frame
 *   2. STOP — do not send the next frame
 *   3. WAIT for ACK from server
 *   4. If ACK received → send next frame
 *      If ACK NOT received → resend the SAME frame (retransmit)
 *
 * Run: ./swclient <port>
 * Ex:  ./swclient 8080
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/* Same structs as the server — both sides must use identical Frame layout
 * so the bytes sent by one side are interpreted correctly by the other.
 *
 *   frame_kind → 0=ACK, 1=SEQ(data), 2=FIN
 *   sq_no      → sequence number of THIS frame
 *   ack        → acknowledgement number (used in ACK frames)
 *   packet     → the actual data */
typedef struct packet {
    char data[1024];
} Packet;

typedef struct frame {
    int frame_kind;
    int sq_no;
    int ack;
    Packet packet;
} Frame;

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    int port = atoi(argv[1]);
    int sockfd;

    struct sockaddr_in serverAddr;
    char buffer[1024];
    socklen_t addr_size;

    int frame_id = 0; /* sequence number of the frame we are currently sending
                         starts at 0, increments after each successful ACK */

    Frame frame_send; /* data frame we send to server */
    Frame frame_recv; /* ACK frame we receive from server */

    int ack_recv = 1; /* flag: 1 = got ACK (ok to send new frame)
                                0 = no ACK  (must resend same frame)
                         starts as 1 so we send the first frame immediately */

    /* ── STEP 1: CREATE UDP SOCKET ───────────────────────────────────────*/
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* ── STEP 2: SET UP SERVER ADDRESS ───────────────────────────────────
     * Client does NOT bind or listen — it just needs to know where to send. */
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* ── STEP 3: MAIN LOOP ────────────────────────────────────────────────*/
    while (1) {

        /* ── SEND PHASE ───────────────────────────────────────────────────
         * Only ask for new data if the last ACK was received (ack_recv == 1).
         * If ack_recv == 0, we skip this block and resend the SAME frame
         * that was already built in the previous iteration. */
        if (ack_recv == 1) {

            /* Build the data frame:
             *   sq_no      = frame_id   → current sequence number
             *   frame_kind = 1          → this is a data (SEQ) frame
             *   ack        = 0          → not an ACK frame, so ack is unused */
            frame_send.sq_no = frame_id;
            frame_send.frame_kind = 1;
            frame_send.ack = 0;

            printf("Enter Data: ");
            scanf("%s", buffer);
            strcpy(frame_send.packet.data, buffer); /* copy typed text into frame */
        }

        /* sendto() sends the frame to the server.
         * If ack_recv was 0, this resends the exact same frame_send as before. */
        sendto(sockfd,
               &frame_send,
               sizeof(Frame),
               0,
               (struct sockaddr *)&serverAddr,
               sizeof(serverAddr));

        printf("[+] Frame %d sent\n", frame_id);

        /* ── WAIT PHASE ───────────────────────────────────────────────────
         * recvfrom() BLOCKS here — client does nothing until ACK arrives.
         * This is the "STOP and WAIT" part of the protocol. */
        addr_size = sizeof(serverAddr);

        int f_recv_size = recvfrom(sockfd,
                                   &frame_recv,
                                   sizeof(frame_recv),
                                   0,
                                   (struct sockaddr *)&serverAddr,
                                   &addr_size);

        /* ── CHECK IF ACK IS VALID ────────────────────────────────────────
         * Three conditions to confirm this is the correct ACK:
         *   1. f_recv_size > 0        → something was actually received
         *   2. frame_recv.sq_no == 0  → it's an ACK frame (sq_no is 0 for ACKs)
         *   3. frame_recv.ack == frame_id + 1
         *                             → server is asking for the NEXT frame
         *                                confirming it received frame_id
         */
        if (f_recv_size > 0 &&
            frame_recv.sq_no == 0 &&
            frame_recv.ack == frame_id + 1) {

            printf("[+] ACK %d received — frame delivered successfully\n", frame_recv.ack);
            ack_recv = 1; /* ACK ok → next iteration will ask for new data */
        }
        else {
            printf("[-] ACK not received — will resend frame %d\n", frame_id);
            ack_recv = 0; /* No ACK → next iteration skips scanf, resends same frame */
        }

        /* Only advance frame_id if ACK was received.
         * If not, frame_id stays the same so the resent frame has the same sq_no. */
        if (ack_recv == 1)
            frame_id++;
    }

    close(sockfd);
    return 0;
}
