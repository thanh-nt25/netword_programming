#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char *message = "Hello from client";
    char ip_address[INET_ADDRSTRLEN];

    printf("Enter server IP address: ");
    scanf("%s", ip_address);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, ip_address, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    // Gửi thông điệp tới server
    write(sock, message, strlen(message));

    // Đọc phản hồi từ server
    int read_bytes = read(sock, buffer, BUFFER_SIZE);
    if (read_bytes < 0) {
        perror("read failed");
        close(sock);
        return -1;
    }

    printf("Response from server: %s\n", buffer);

    close(sock);

    return 0;
}

