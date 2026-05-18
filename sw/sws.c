/* STOP AND WAIT - CLIENT */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 8082

typedef struct {
    int seq, ack, type;
    char data[1024];
} Frame;

int main() {
    int sockfd, frame_id = 0, need_new = 1;
    struct sockaddr_in server;
    socklen_t len = sizeof(server);
    Frame sf, rf;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family      = AF_INET;
    server.sin_port        = htons(PORT);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1) {
        if (need_new) {
            sf.type = 1;
            sf.seq  = frame_id;
            sf.ack  = 0;
            printf("Enter data: ");
            scanf("%s", sf.data);
        }

        sendto(sockfd, &sf, sizeof(sf), 0, (struct sockaddr *)&server, sizeof(server));
        printf("[+] Frame sent\n");

        recvfrom(sockfd, &rf, sizeof(rf), 0, (struct sockaddr *)&server, &len);

        if (rf.ack == frame_id + 1) {
            printf("[+] ACK received\n");
            frame_id++;
            need_new = 1;
        } else {
            printf("[-] ACK not received — resending\n");
            need_new = 0;
        }
    }

    return 0;
}
