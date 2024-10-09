#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int sock;
    struct sockaddr_in server_addr;
    char buffer[1024];

    // Tao socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // Connect den server
    connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));

    for (int i = 0; i < 10; i++) {
        recv(sock, buffer, sizeof(buffer), 0); 
        printf("Question %d: %s\n", i + 1, buffer);

        recv(sock, buffer, sizeof(buffer), 0); 
        printf("Options:\n");
        for (int j = 0; j < 4; j++) {
            printf("%d. %s\n", j + 1, ((char(*)[256])buffer)[j]);
        }

        // tra loi
        printf("Your answer (1-4): ");
        int answer;
        scanf("%d", &answer);
        printf("cau tra loi vua nhap: %d\n", answer);
        send(sock, &answer, sizeof(answer), 0); // gui cau tra loi
    }

    recv(sock, buffer, sizeof(buffer), 0);
    printf("%s\n", buffer);

    close(sock);
    return 0;
}
