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
    char input[BUFFER_SIZE];

    // Tạo socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Chuyển đổi địa chỉ IP
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("Invalid address or address not supported\n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
    }

    // Yêu cầu người dùng nhập chuỗi
    printf("Enter a string to send to the server: ");
    fgets(input, BUFFER_SIZE, stdin);

    // Xóa ký tự xuống dòng ở cuối chuỗi (nếu có)
    input[strcspn(input, "\n")] = '\0';

    // Gửi chuỗi đến server
    send(sock, input, strlen(input), 0);
    printf("Message sent to server: %s\n", input);

    // Đọc chuỗi đã được viết hoa từ server
    read(sock, buffer, BUFFER_SIZE);
    printf("Message from server (uppercase): %s\n", buffer);

    // Đóng kết nối
    close(sock);

    return 0;
}
