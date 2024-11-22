// mem-check-client.c
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
    char *message = "Hello from client";
    struct sockaddr_in servaddr, recv_servaddr;
    char *server_ip = "127.0.0.1";  // Specify the server IP address

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&recv_servaddr, 0, sizeof(recv_servaddr));

    // Filling server information with specific IP address and port
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);  // Specify server port number
    
    // Convert the IP address from text to binary form
    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        exit(EXIT_FAILURE);
    }

    // Send message to server
    sendto(sockfd, (const char *)message, strlen(message), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    printf("Message sent to server at IP %s and Port %d.\n", server_ip, PORT);

    // Receive acknowledgment from server
    int n;
    socklen_t len = sizeof(recv_servaddr);
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, 0, (struct sockaddr *)&recv_servaddr, &len);
    buffer[n] = '\0';

    // Convert the received server's IP address from binary to text format
    char recv_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &recv_servaddr.sin_addr, recv_ip, INET_ADDRSTRLEN);

    // Compare the entire sockaddr_in structure (both IP address and port)
    if (memcmp(&servaddr, &recv_servaddr, sizeof(struct sockaddr_in)) == 0) {
        printf("Received message from the correct server (IP: %s, Port: %d).\n", recv_ip, ntohs(recv_servaddr.sin_port));
        printf("Server message: %s\n", buffer);
    } else {
        printf("Received message from an unexpected server (IP: %s, Port: %d).\n", recv_ip, ntohs(recv_servaddr.sin_port));
    }

    close(sockfd);
    return 0;
}
