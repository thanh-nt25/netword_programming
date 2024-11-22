// udp_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define MAXLINE 1024

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information
    servaddr.sin_family = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    int n;
    socklen_t len = sizeof(cliaddr); // len is value/result

    printf("Server is running...\n");

    // Receive message from client
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
    buffer[n] = '\0';

    // Convert client's IP address from network byte order to human-readable format
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(cliaddr.sin_addr), client_ip, INET_ADDRSTRLEN);

    // Display the client's message, IP address, and port number
    printf("Client IP: %s, Port: %d\n", client_ip, ntohs(cliaddr.sin_port));
    printf("Client message: %s\n", buffer);

    // Send acknowledgment to client
    char *ack = "Hello from server";
    sendto(sockfd, ack, strlen(ack), 0, (const struct sockaddr *)&cliaddr, len); // Replaced MSG_CONFIRM with 0
    printf("Acknowledgment message sent to client.\n");

    close(sockfd);
    return 0;
}
