/*
 * STOP AND WAIT ARQ — CLIENT SIDE
 *
 * Sends frames to the server one at a time.
 * After sending each frame, STOPS and WAITS for an ACK.
 * If ACK is correct  → send the next frame (new data)
 * If ACK is wrong    → resend the same frame again (retransmit)
 *
 * Uses UDP — no connection needed, just send/receive packets.
 *
 * How to run:  ./sw_v2_client
 */

#include <stdio.h>       /* printf, scanf */
#include <stdlib.h>      /* exit */
#include <string.h>      /* memset, strcpy */
#include <sys/socket.h>  /* socket, sendto, recvfrom */
#include <arpa/inet.h>   /* htons, inet_addr */

#define PORT 8080

/* Frame is the envelope sent between client and server.
 *   seq  → sequence number: which frame is this? (0, 1, 2, 3...)
 *   ack  → acknowledgement number: set by server in ACK reply
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
    int frame_id = 0;              /* sequence number of the frame we are sending
                                      starts at 0, only increments after correct ACK */
    int need_new_data = 1;         /* flag that controls whether to ask user for input
                                        1 = last frame was ACKed → get new message
                                        0 = no ACK received     → resend same frame */
    struct sockaddr_in server;     /* server's address — where to send frames */
    socklen_t len = sizeof(server);
    Frame send_frame;              /* frame we send TO server */
    Frame recv_frame;              /* ACK frame we receive FROM server */

    /* ── STEP 1: CREATE A UDP SOCKET ────────────────────────────────────────
     * SOCK_DGRAM = UDP. Client does NOT call bind() or listen() —
     * it only needs to know where to send (server address). */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* ── STEP 2: FILL IN THE SERVER'S ADDRESS ───────────────────────────────
     *   sin_family       → AF_INET = IPv4
     *   sin_port         → htons converts port to network byte order
     *   inet_addr()      → converts "127.0.0.1" string to binary IP number */
    memset(&server, 0, sizeof(server));
    server.sin_family      = AF_INET;
    server.sin_port        = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* ── STEP 3: SEND/WAIT LOOP ──────────────────────────────────────────────
     * Each iteration either sends a NEW frame or RESENDS the previous one. */
    while (1) {

        /* ── STEP 4: BUILD THE FRAME (only if last one was ACKed) ────────────
         * need_new_data == 1 → previous frame was delivered → ask for new input
         * need_new_data == 0 → no ACK came back            → skip scanf,
         *                       send_frame still holds the previous data,
         *                       so the same frame gets resent automatically */
        if (need_new_data) {
            send_frame.type = 1;         /* 1 = this is a data frame */
            send_frame.seq  = frame_id;  /* stamp with current sequence number */
            send_frame.ack  = 0;         /* ack unused in data frames */

            printf("Enter data: ");
            scanf("%s", send_frame.data); /* read message from keyboard */
        }

        /* ── STEP 5: SEND THE FRAME ──────────────────────────────────────────
         * sendto() — UDP version of send().
         * Must specify destination every time (no persistent connection in UDP).
         *
         * Arguments:
         *   sockfd       → our socket
         *   &send_frame  → the frame to send
         *   sizeof(Frame)→ how many bytes to send
         *   0            → no flags
         *   &server      → destination: server's IP and port
         *   sizeof(server) → size of the address struct */
        sendto(sockfd, &send_frame, sizeof(Frame), 0,
               (struct sockaddr *)&server, sizeof(server));

        printf("[+] Frame %d sent\n", frame_id);

        /* ── STEP 6: STOP — WAIT FOR ACK ─────────────────────────────────────
         * recvfrom() BLOCKS here — client does nothing until server replies.
         * This blocking wait is the "STOP and WAIT" in the protocol name.
         * Client is stuck here until an ACK (or wrong reply) arrives. */
        recvfrom(sockfd, &recv_frame, sizeof(Frame), 0,
                 (struct sockaddr *)&server, &len);

        /* ── STEP 7: CHECK IF ACK IS CORRECT ─────────────────────────────────
         * Server sets recv_frame.ack = frame_id + 1 when it gets our frame.
         * So we check: did server ask for the NEXT frame?
         *
         *   recv_frame.ack == frame_id + 1
         *     TRUE  → server got our frame → move to next frame
         *     FALSE → something went wrong → resend same frame */
        if (recv_frame.ack == frame_id + 1) {

            printf("[+] ACK %d received — frame delivered\n", recv_frame.ack);
            frame_id++;        /* advance to next sequence number */
            need_new_data = 1; /* ask user for new input next iteration */
        }
        else {

            printf("[-] Wrong ACK — resending frame %d\n", frame_id);
            need_new_data = 0; /* skip scanf next iteration, resend same frame */
        }

        /* frame_id only goes up when ACK is correct.
         * If no ACK: frame_id stays the same, same frame resent with same seq number.
         * Server will recognize the seq number and ACK it again. */
    }

    return 0;
}
