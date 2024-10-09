#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 2048

int clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Thêm client vào danh sách
void add_client(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    clients[client_count++] = client_socket;
    pthread_mutex_unlock(&clients_mutex);
}

// Xóa client khỏi danh sách
void remove_client(int client_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] == client_socket) {
            for (int j = i; j < client_count - 1; ++j) {
                clients[j] = clients[j + 1];
            }
            break;
        }
    }
    --client_count;
    pthread_mutex_unlock(&clients_mutex);
}

// Gửi tin nhắn tới tất cả các client khác
void broadcast_message(char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < client_count; ++i) {
        if (clients[i] != sender_socket) {
            if (send(clients[i], message, strlen(message), 0) < 0) {
                perror("Sending message error");
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Xử lý từng client
void *handle_client(void *arg) {
    int client_socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    int read_size;

    // Thêm client mới
    add_client(client_socket);

    while ((read_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        broadcast_message(buffer, client_socket);  // Phát tin nhắn
    }

    // Client ngắt kết nối
    remove_client(client_socket);
    close(client_socket);

    return NULL;
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t tid;

    // Tạo socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Cannot create socket");
        return 1;
    }

    // Gán địa chỉ và cổng
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    // Gán địa chỉ cho socket
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind Error");
        return 1;
    }

    // Lắng nghe kết nối
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listening Error");
        return 1;
    }

    printf("Server is listening on Port 8080...\n");

    while (1) {
        // Chấp nhận kết nối từ client
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_socket < 0) {
            perror("Cannot accpet connection");
            return 1;
        }

        // Tạo thread để xử lý client
        if (pthread_create(&tid, NULL, handle_client, &client_socket) != 0) {
            perror("Cannot create thread");
            return 1;
        }
    }

    close(server_socket);
    return 0;
}
