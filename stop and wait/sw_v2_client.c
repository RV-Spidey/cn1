/* STOP AND WAIT - CLIENT (simplest version) */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8080

typedef struct {
    int seq;
    int ack;
    int type;
    char data[1024];
} Frame;

int main() {

    int sockfd;
    int frame_id = 0; /* sequence number of current frame */
    struct sockaddr_in server;
    socklen_t len = sizeof(server);
    Frame send_frame, recv_frame;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    int need_new_data = 1; /* 1 = ask for new input, 0 = resend same frame */

    while (1) {

        /* only ask for new data if last frame was acknowledged */
        if (need_new_data) {
            send_frame.type = 1;
            send_frame.seq  = frame_id;
            send_frame.ack  = 0;

            printf("Enter data: ");
            scanf("%s", send_frame.data);
        }

        sendto(sockfd, &send_frame, sizeof(Frame), 0,
               (struct sockaddr *)&server, sizeof(server));

        printf("[+] Frame %d sent\n", frame_id);

        /* STOP — wait for ACK before sending next frame */
        recvfrom(sockfd, &recv_frame, sizeof(Frame), 0,
                 (struct sockaddr *)&server, &len);

        if (recv_frame.ack == frame_id + 1) {
            printf("[+] ACK %d received\n", recv_frame.ack);
            frame_id++;
            need_new_data = 1; /* got ACK — ask for new input next */
        }
        else {
            printf("[-] Wrong ACK — resending frame %d\n", frame_id);
            need_new_data = 0; /* no ACK — resend same frame, skip scanf */
        }
    }

    return 0;
}
