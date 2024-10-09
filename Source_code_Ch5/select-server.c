#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8080
#define BUFFER_SIZE 100

int main() {
    int sockfd;
    struct sockaddr_in server_addr;
    fd_set read_fds;
    struct timeval timeout;
    char buffer[BUFFER_SIZE];

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(sockfd, 1) < 0) {
        perror("listen");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Accept a connection (blocking call)
    int client_sock = accept(sockfd, NULL, NULL);
    if (client_sock < 0) {
        perror("accept");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected.\n");

    while (1) {
        // Initialize the file descriptor set
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds); // Monitor standard input
        FD_SET(client_sock, &read_fds);  // Monitor the client socket

        // Set the timeout to 5 seconds
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        // Determine the maximum file descriptor value
        int max_fd = client_sock > STDIN_FILENO ? client_sock : STDIN_FILENO;

        // Use select to wait for input or timeout
        int result = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

        if (result == -1) {
            perror("select");
            break;
        } else if (result == 0) {
            // Timeout occurred
            printf("Timeout occurred! No data after 5 seconds.\n");
        } else {
            // Check for data on standard input
            if (FD_ISSET(STDIN_FILENO, &read_fds)) {
                printf("Enter some text: ");
                fgets(buffer, sizeof(buffer), stdin);
                // Optionally send the input to the client
                send(client_sock, buffer, strlen(buffer), 0);
                printf("You entered: %s", buffer);
            }

            // Check for data from the client socket
            if (FD_ISSET(client_sock, &read_fds)) {
                int bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
                if (bytes_received <= 0) {
                    // Client disconnected or error
                    perror("recv");
                    break;
                }
                buffer[bytes_received] = '\0'; // Null-terminate the received data
                printf("Received from client: %s", buffer);
            }
        }
    }

    // Clean up
    close(client_sock);
    close(sockfd);
    return 0;
}
