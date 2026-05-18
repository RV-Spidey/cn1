/* STOP AND WAIT - SERVER (simplest version) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080

/* Single flat struct — no nesting, easy to understand
 *   seq  = sequence number of this frame
 *   ack  = acknowledgement number (used in ACK reply)
 *   type = 1 means data frame, 0 means ACK frame
 *   data = the actual message */
typedef struct {
    int seq;
    int ack;
    int type;
    char data[1024];
} Frame;

int main() {

    int sockfd;
    int expected = 0; /* sequence number we expect from client next */
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);
    Frame recv_frame, ack_frame;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr *)&server, sizeof(server));
    printf("Server ready on port %d...\n", PORT);

    while (1) {

        /* wait for a frame from client */
        recvfrom(sockfd, &recv_frame, sizeof(Frame), 0,
                 (struct sockaddr *)&client, &len);

        if (recv_frame.seq == expected) {
            /* correct frame — print data and send ACK */
            printf("[+] Received frame %d: %s\n", recv_frame.seq, recv_frame.data);

            ack_frame.type = 0;            /* 0 = ACK frame */
            ack_frame.seq  = 0;
            ack_frame.ack  = expected + 1; /* "I got frame N, send N+1 next" */

            sendto(sockfd, &ack_frame, sizeof(ack_frame), 0,
                   (struct sockaddr *)&client, len);

            printf("[+] ACK %d sent\n", ack_frame.ack);
            expected++;
        }
        else {
            printf("[-] Wrong frame (got %d, expected %d)\n", recv_frame.seq, expected);
        }
    }

    return 0;
}
