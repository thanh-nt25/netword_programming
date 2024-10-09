#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h> // For error codes

#define PORT 8080
#define SERVER_ADDR "192.168.1.100" // Example IP, make sure to set this to a reachable/unreachable server

int main() {
    int sock;
    struct sockaddr_in server_address;

    // Step 1: Create a socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Step 2: Define the server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    
    // Convert IP address to binary form and set it in server_address
    if (inet_pton(AF_INET, SERVER_ADDR, &server_address.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Step 3: Attempt to connect to the server
    if (connect(sock, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        // Analyze the error code using errno
        switch (errno) {
            case ETIMEDOUT:
                printf("Connection timed out (ETIMEDOUT)\n");
                break;
            case ECONNREFUSED:
                printf("Connection refused by the server (ECONNREFUSED)\n");
                break;
            case EHOSTUNREACH:
                printf("No route to the host (EHOSTUNREACH)\n");
                break;
            case ENETUNREACH:
                printf("Network is unreachable (ENETUNREACH)\n");
                break;
            case EADDRNOTAVAIL:
                printf("Invalid local IP address (EADDRNOTAVAIL)\n");
                break;
            case EINPROGRESS:
                printf("Connection attempt in progress (EINPROGRESS)\n");
                break;
            default:
                printf("Connection failed with error code: %d (%s)\n", errno, strerror(errno));
                break;
        }
        close(sock);
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server successfully!\n");

    // Step 4: Close the socket when done
    close(sock);

    return 0;
}
