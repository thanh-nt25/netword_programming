#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>  // For socket functions, sockaddr_in, inet_pton

int main() {
    int sock;
    struct sockaddr_in server_addr;

    // Message to send to server
    char *message = "Hello, Server!";
    
    // Create a socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        return -1;
    }

    // Server address configuration
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);  // Server port number

    // Convert IPv4 address from text to binary and store in sin_addr
    if (inet_pton(AF_INET, "192.168.1.1", &server_addr.sin_addr) <= 0) {
        printf("Invalid address/Address not supported\n");
        return -1;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return -1;
    }

    printf("Connected to the server successfully\n");

    // Send a message to the server
    send(sock, message, strlen(message), 0);
    printf("Message sent to server: %s\n", message);

    // Close the socket
    close(sock);

    return 0;
}
