#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define MAXLINE 1024
#define TIMEOUT 5 // 5 seconds

void xor_cipher(char *data, char key) {
    for (int i = 0; data[i] != '\0'; i++) {
        data[i] ^= key;
    }
}

int main() {
    int sockfd;
    char buffer[MAXLINE];
    // char *message = "Hello from client";
    struct sockaddr_in servaddr;
    fd_set readfds;
    struct timeval tv;
    char key = 'K'; // Key for XOR encryption

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    char message[MAXLINE];
    printf("Enter message send to server: ");
    fgets(message, MAXLINE, stdin);  // Nhận chuỗi từ bàn phím
    message[strcspn(message, "\n")] = 0;

    // Send message to server
    sendto(sockfd, message, strlen(message), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Message sent to server.\n");

    // Wait for server response using select()
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);

    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;

    int activity = select(sockfd + 1, &readfds, NULL, NULL, &tv);

    if (activity < 0) {
        perror("Select error");
    } else if (activity == 0) {
        printf("Timeout, no response received in %d seconds.\n", TIMEOUT);
        close(sockfd);
        return 0;
    }

    // Receive encrypted message from server
    socklen_t len = sizeof(servaddr);
    int n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&servaddr, &len);
    buffer[n] = '\0';

    // Decrypt the message
    xor_cipher(buffer, key);
    printf("Decrypted message from server: %s\n", buffer);

    close(sockfd);
    return 0;
}
