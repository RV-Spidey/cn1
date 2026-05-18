/* STOP AND WAIT - SERVER */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

typedef struct packet { char data[1024]; } Packet;
typedef struct frame { int frame_kind; int sq_no; int ack; Packet packet; } Frame;

int main(int argc, char *argv[]) {

    if (argc != 2) { printf("Usage: %s <port>\n", argv[0]); exit(0); }

    int sockfd;
    int frame_id = 0;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addr_size;
    Frame frame_recv, frame_send;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(atoi(argv[1]));
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

    printf("Server ready...\n");

    addr_size = sizeof(clientAddr);

    while (1) {

        int f_recv_size = recvfrom(sockfd, &frame_recv, sizeof(Frame), 0,
                                   (struct sockaddr *)&clientAddr, &addr_size);

        if (f_recv_size > 0 && frame_recv.frame_kind == 1 && frame_recv.sq_no == frame_id) {

            printf("[+] Frame %d received: %s\n", frame_recv.sq_no, frame_recv.packet.data);

            frame_send.frame_kind = 0;
            frame_send.sq_no = 0;
            frame_send.ack = frame_recv.sq_no + 1;

            sendto(sockfd, &frame_send, sizeof(frame_send), 0,
                   (struct sockaddr *)&clientAddr, addr_size);

            printf("[+] ACK %d sent\n", frame_send.ack);
        }
        else {
            printf("[-] Frame %d not received correctly\n", frame_id);
        }

        frame_id++;
    }

    close(sockfd);
    return 0;
}
