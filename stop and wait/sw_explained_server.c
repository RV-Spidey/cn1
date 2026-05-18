/*
 * STOP AND WAIT ARQ — SERVER (EXPLAINED)
 *
 * Stop-and-Wait is a simple protocol where:
 *   - Client sends ONE frame and STOPS
 *   - Server receives it, sends back an ACK (acknowledgement)
 *   - Client WAITS for that ACK before sending the next frame
 *
 * Uses UDP (SOCK_DGRAM) — unlike TCP, UDP does NOT guarantee delivery,
 * so Stop-and-Wait adds that reliability manually using ACKs.
 *
 * Run: ./swserver <port>
 * Ex:  ./swserver 8080
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

/* ── STRUCTS ─────────────────────────────────────────────────────────────────
 * Packet: holds the actual message data (up to 1024 chars)
 * Frame:  the envelope around the packet — adds control info:
 *
 *   frame_kind → what type of frame is this?
 *                  0 = ACK  (acknowledgement — "I got it")
 *                  1 = SEQ  (data frame — "here is data")
 *                  2 = FIN  (finish — "I'm done")
 *
 *   sq_no      → sequence number (0, 1, 2, 3...)
 *                tells the receiver which frame this is
 *                prevents duplicate frames being accepted twice
 *
 *   ack        → acknowledgement number
 *                server sets this to sq_no + 1
 *                meaning "I received up to sq_no, send sq_no+1 next"
 *
 *   packet     → the actual data payload inside the frame
 */
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

    int port = atoi(argv[1]); /* convert port string "8080" to integer 8080 */
    int sockfd;

    struct sockaddr_in serverAddr; /* server's own address */
    struct sockaddr_in clientAddr; /* will be filled with client's address by recvfrom */

    socklen_t addr_size;

    int frame_id = 0; /* the sequence number we EXPECT from the client next
                         starts at 0, increments after each successful receive */

    Frame frame_recv; /* frame received FROM client */
    Frame frame_send; /* ACK frame we send BACK to client */

    /* ── STEP 1: CREATE UDP SOCKET ───────────────────────────────────────
     * SOCK_DGRAM = UDP (connectionless, no handshake)
     * Unlike TCP, UDP just sends/receives packets with no connection setup.
     * Stop-and-Wait adds reliability ON TOP of UDP manually. */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    /* ── STEP 2: SET UP SERVER ADDRESS ───────────────────────────────────*/
    memset(&serverAddr, '\0', sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    /* ── STEP 3: BIND ─────────────────────────────────────────────────────
     * Reserve this port so incoming UDP packets reach this program. */
    bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    printf("Server waiting for frames on port %d...\n", port);

    addr_size = sizeof(clientAddr);

    /* ── STEP 4: MAIN LOOP ────────────────────────────────────────────────
     * Keep receiving frames forever. */
    while (1) {

        /* recvfrom() — like recv() but for UDP.
         * UDP has no connection, so we don't know WHO sent the packet.
         * recvfrom() fills clientAddr with the sender's IP + port
         * so we know where to send the ACK back.
         *
         * Arguments:
         *   sockfd       → our socket
         *   &frame_recv  → where to store the incoming frame
         *   sizeof(Frame)→ max bytes to read
         *   0            → no flags
         *   &clientAddr  → OUTPUT: filled with sender's address
         *   &addr_size   → size of clientAddr struct */
        int f_recv_size = recvfrom(sockfd,
                                   &frame_recv,
                                   sizeof(Frame),
                                   0,
                                   (struct sockaddr *)&clientAddr,
                                   &addr_size);

        /* ── CHECK IF FRAME IS VALID ──────────────────────────────────────
         * Three conditions must ALL be true to accept the frame:
         *   1. f_recv_size > 0      → something was actually received
         *   2. frame_kind == 1      → it's a data frame (SEQ), not an ACK
         *   3. sq_no == frame_id    → it's the frame we EXPECTED
         *                             (not a duplicate or out-of-order frame)
         */
        if (f_recv_size > 0 &&
            frame_recv.frame_kind == 1 &&
            frame_recv.sq_no == frame_id) {

            printf("[+] Frame %d received: %s\n", frame_recv.sq_no, frame_recv.packet.data);

            /* ── SEND ACK BACK ────────────────────────────────────────────
             * Build the ACK frame:
             *   frame_kind = 0           → this is an ACK frame
             *   sq_no      = 0           → not a data frame, so sq_no irrelevant
             *   ack        = sq_no + 1   → "I got frame N, now send frame N+1"
             */
            frame_send.frame_kind = 0;
            frame_send.sq_no = 0;
            frame_send.ack = frame_recv.sq_no + 1;

            /* sendto() — UDP version of send()
             * We must specify the destination (clientAddr) every time
             * because UDP has no persistent connection. */
            sendto(sockfd,
                   &frame_send,
                   sizeof(frame_send),
                   0,
                   (struct sockaddr *)&clientAddr,
                   addr_size);

            printf("[+] ACK %d sent\n", frame_send.ack);
        }
        else {
            /* Frame was corrupted, wrong sequence number, or wrong type.
             * In a real implementation we would resend the last ACK here.
             * For now we just print and move on. */
            printf("[-] Frame %d not received correctly\n", frame_id);
        }

        /* Move to next expected sequence number */
        frame_id++;
    }

    close(sockfd);
    return 0;
}
