#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <poll.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048
#define PORT 8080

int clients[MAX_CLIENTS];
int client_count = 0;
int server_socket;

void handle_signal(int signal) {
    if (signal == SIGINT) {
        printf("\nDisconnected server...\n");
        for (int i = 0; i < client_count; i++) {
            close(clients[i]);
        }
        close(server_socket);
        exit(0);
    }
}

void broadcast_message(int sender_fd, int *client_sockets, int num_clients, char *message) {
    for (int i = 0; i < num_clients; i++) {
        int client_fd = client_sockets[i];
        if (client_fd != sender_fd && client_fd > 0) {
            send(client_fd, message, strlen(message), 0);
        }
    }
}

void handle_clients() {
    struct pollfd fds[MAX_CLIENTS + 1];  
    int timeout = -1;  
    int new_socket;

    fds[0].fd = server_socket;
    fds[0].events = POLLIN;

    while (1) {
        for (int i = 0; i < client_count; i++) {
            fds[i + 1].fd = clients[i];
            fds[i + 1].events = POLLIN;
        }

        int activity = poll(fds, client_count + 1, timeout);
        if (activity < 0) {
            perror("pselect error");
            continue;
        }

        if (fds[0].revents & POLLIN) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
            if (new_socket < 0) {
                perror("Cannot accept connection");
                continue;
            }

            if (client_count < MAX_CLIENTS) {
                clients[client_count++] = new_socket;
                printf("New client connected: socket %d\n", new_socket);
            } else {
                printf("Max client reached\n");
                close(new_socket);
            }
        }

        for (int i = 0; i < client_count; i++) {
            if (fds[i + 1].revents & POLLIN) {
                char buffer[BUFFER_SIZE];
                int read_size = recv(fds[i + 1].fd, buffer, BUFFER_SIZE, 0);
                if (read_size <= 0) {
                    close(fds[i + 1].fd);
                    printf("Client %d disconnected\n", clients[i]);

                    for (int j = i; j < client_count - 1; j++) {
                        clients[j] = clients[j + 1];
                    }
                    client_count--;
                    i--;  
                } else {
                    buffer[read_size] = '\0';
                    printf("New message from client %d: %s", clients[i], buffer);
                    broadcast_message(fds[i + 1].fd, clients, client_count, buffer);  // Phát tin nhắn
                }
            }
        }
    }
}

int main() {
    struct sockaddr_in server_addr;

    signal(SIGINT, handle_signal);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Cannot create socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind error");
        return 1;
    }

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listening error");
        return 1;
    }

    printf("Poll Server is listening on port %d...\n", PORT);

    handle_clients();

    for (int i = 0; i < client_count; i++) {
        close(clients[i]);
    }
    close(server_socket);
    return 0;
}
