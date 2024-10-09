// client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int n;

    // Create a socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server. Type a message and press enter:\n");

    while (1) {
        // Get input from the user
        fgets(buffer, BUFFER_SIZE, stdin);

        // Remove the newline character if it's present (fgets adds it)
        if (buffer[0] != '\0' && buffer[strlen(buffer) - 1] == '\n') {
            buffer[strlen(buffer) - 1] = '\0';
        }

        // Check if the string is empty. If it is, stop the program
        if (buffer[0] == '\0') {
            break;
        }
        // Send the message to the server
        send(client_socket, buffer, strlen(buffer), 0);

        // Receive the echo back from the server
        n = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (n > 0) {
            buffer[n] = '\0'; // Null-terminate the received string
            printf("Server: %s\n", buffer);
        } else if (n == 0) {
            printf("Server disconnected.\n");
            break;
        } else {
            perror("recv failed");
            break;
        }
    }

    // Close the socket
    close(client_socket);

    return 0;
}
