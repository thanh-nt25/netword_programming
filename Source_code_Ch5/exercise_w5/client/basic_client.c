#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 2048
char username[50];

// Nhận tin nhắn từ server
void *receive_messages(void *arg) {
    int socket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    int read_size;

    while ((read_size = recv(socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0';
        printf("%s", buffer);
    }

    return NULL;
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char message[BUFFER_SIZE];
    pthread_t tid;
    
    // Tạo socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Cannot create socket");
        return 1;
    }

    // Thiết lập địa chỉ server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("192.168.100.4");  // Địa chỉ IP của server
    server_addr.sin_port = htons(8080);  // Cổng của server

    // Kết nối tới server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect to server fails\n");
        return 1;
    }

    printf("Enter your name: ");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;

    // Tạo thread để nhận tin nhắn từ server
    if (pthread_create(&tid, NULL, receive_messages, &client_socket) != 0) {
        perror("Cannot create thread");
        return 1;
    }

    // Gửi tin nhắn
    while (1) {
        fgets(message, BUFFER_SIZE, stdin);
        // Thêm tên người dùng vào tin nhắn
        // char full_message[BUFFER_SIZE];
        // snprintf(full_message, sizeof(full_message), "%s: %s", username, message);
        
        if (send(client_socket, message, strlen(message), 0) < 0) {
            perror("Sending message error!\n");
            return 1;
        }
    }

    close(client_socket);
    return 0;
}
