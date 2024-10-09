#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define SERVER_ADDR "127.0.0.1"
#define BUFFER_SIZE 1024

int main() {
    int sock;
    struct sockaddr_in server_address;
    char buffer[BUFFER_SIZE] = {0};
    const char *message = "Hello, Server!";
    
    // Step 1: Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Step 2: Define the server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    
    // Convert IP address to binary form and set it in server_address
    if (inet_pton(AF_INET, SERVER_ADDR, &server_address.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sock);
        return -1;
    }

    // Step 3: Connect to the server
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Connection to the server failed");
        close(sock);
        return -1;
    }

    // Step 4: Send a message to the server
    send(sock, message, strlen(message), 0);
    printf("Message sent: %s\n", message);

    // Step 5: Receive a response from the server
    int bytes_received = recv(sock, buffer, BUFFER_SIZE, 0);
    if (bytes_received < 0) {
        perror("Receive failed");
    } else {
        printf("Server response: %s\n", buffer);
    }

    // Step 6: Close the socket
    close(sock);
    
    return 0;
}
