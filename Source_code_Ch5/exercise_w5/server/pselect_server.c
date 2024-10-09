#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048

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

void broadcast_message(char *message, int sender_socket) {
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] != sender_socket) {
            if (send(clients[i], message, strlen(message), 0) < 0) {
                perror("Sending message error");
            }
        }
    }
}

void handle_clients() {
    fd_set read_fds;  
    int max_sd;

    while (1) {
        FD_ZERO(&read_fds);

        FD_SET(server_socket, &read_fds);
        max_sd = server_socket;

        for (int i = 0; i < client_count; ++i) {
            FD_SET(clients[i], &read_fds);
            if (clients[i] > max_sd) {
                max_sd = clients[i];
            }
        }

        int activity = pselect(max_sd + 1, &read_fds, NULL, NULL, NULL, NULL);
        if (activity < 0) {
            perror("pselect error");
            continue;
        }

        if (FD_ISSET(server_socket, &read_fds)) {
            struct sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int new_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
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

        for (int i = 0; i < client_count; ++i) {
            if (FD_ISSET(clients[i], &read_fds)) {
                char buffer[BUFFER_SIZE];
                int read_size = recv(clients[i], buffer, BUFFER_SIZE, 0);
                if (read_size <= 0) {
                    close(clients[i]);
                    printf("Client %d disconnected\n", clients[i]);
                    for (int j = i; j < client_count - 1; ++j) {
                        clients[j] = clients[j + 1];
                    }
                    client_count--;
                    i--;
                } else {
                    buffer[read_size] = '\0';
                    printf("New message from client %d: %s", clients[i], buffer);
                    broadcast_message(buffer, clients[i]); 
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
    server_addr.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind error");
        return 1;
    }

    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listening error");
        return 1;
    }


    handle_clients();

    for (int i = 0; i < client_count; i++) {
        close(clients[i]);
    }
    close(server_socket);
    return 0;
}
