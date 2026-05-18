/* STOP AND WAIT - CLIENT */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

typedef struct packet { char data[1024]; } Packet;
typedef struct frame { int frame_kind; int sq_no; int ack; Packet packet; } Frame;

int main(int argc, char *argv[]) {

    if (argc != 2) { printf("Usage: %s <port>\n", argv[0]); exit(0); }

    int sockfd;
    int frame_id = 0;
    int ack_recv = 1;
    struct sockaddr_in serverAddr;
    socklen_t addr_size;
    char buffer[1024];
    Frame frame_send, frame_recv;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[1]));
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1) {

        if (ack_recv == 1) {
            frame_send.sq_no = frame_id;
            frame_send.frame_kind = 1;
            frame_send.ack = 0;

            printf("Enter Data: ");
            scanf("%s", buffer);
            strcpy(frame_send.packet.data, buffer);
        }

        sendto(sockfd, &frame_send, sizeof(Frame), 0,
               (struct sockaddr *)&serverAddr, sizeof(serverAddr));

        printf("[+] Frame %d sent\n", frame_id);

        addr_size = sizeof(serverAddr);

        int f_recv_size = recvfrom(sockfd, &frame_recv, sizeof(frame_recv), 0,
                                   (struct sockaddr *)&serverAddr, &addr_size);

        if (f_recv_size > 0 && frame_recv.sq_no == 0 && frame_recv.ack == frame_id + 1) {
            printf("[+] ACK %d received\n", frame_recv.ack);
            ack_recv = 1;
        }
        else {
            printf("[-] ACK not received — resending frame %d\n", frame_id);
            ack_recv = 0;
        }

        if (ack_recv == 1)
            frame_id++;
    }

    close(sockfd);
    return 0;
}
