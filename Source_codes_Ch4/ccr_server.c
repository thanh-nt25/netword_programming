// concurrent_server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Signal handler to prevent zombie processes
void sigchld_handler(int sig) {
    (void)sig; // Ignore unused parameter warning
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_client(int connfd) {
    char buffer[BUFFER_SIZE];
    int n;

    // Communication with client
    while ((n = recv(connfd, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[n] = '\0'; // Null-terminate the received string
        printf("Client: %s\n", buffer);

        // Echo back the message to the client
        send(connfd, buffer, n, 0);
    }

    if (n == 0) {
        printf("Client disconnected.\n");
    } else {
        perror("recv failed");
    }

    // Close the client socket
    close(connfd);
}

int main() {
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    pid_t pid;

    // Create the listening socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Setup the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the listening socket to the specified port
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 5) < 0) {
        perror("Listen failed");
        close(listenfd);
        exit(EXIT_FAILURE);
    }

    // Handle SIGCHLD to prevent zombie processes
    signal(SIGCHLD, sigchld_handler);

    printf("Server is listening on port %d...\n", PORT);

    // Server loop to accept multiple clients
    while (1) {
        // Accept an incoming connection
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addr_len);
        if (connfd < 0) {
            perror("Accept failed");
            continue;
        }

        // Fork a child process to handle the client
        pid = fork();
        if (pid < 0) {
            perror("Fork failed");
            close(connfd);
        } else if (pid == 0) {
            // Child process: handle the client
            close(listenfd);  // Close the listening socket in the child process
            handle_client(connfd);
            close(connfd);
            exit(0);
        } else {
            // Parent process: continue accepting new clients
            close(connfd);  // Close the client socket in the parent process
        }
    }

    return 0;
}
