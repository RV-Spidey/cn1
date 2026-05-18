/* STOP AND WAIT - SERVER */

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
    int sockfd, expected = 0;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);
    Frame rf, af;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family      = AF_INET;
    server.sin_port        = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(sockfd, (struct sockaddr *)&server, sizeof(server));
    printf("Server ready...\n");

    while (1) {
        recvfrom(sockfd, &rf, sizeof(rf), 0, (struct sockaddr *)&client, &len);

        if (rf.seq == expected) {
            printf("[+] Frame received: %s\n", rf.data);

            af.type = 0;
            af.seq  = 0;
            af.ack  = expected + 1;

            sendto(sockfd, &af, sizeof(af), 0, (struct sockaddr *)&client, len);
            printf("[+] ACK sent\n");

            expected++;
        } else {
            printf("[-] Wrong frame\n");
        }
    }

    return 0;
}
