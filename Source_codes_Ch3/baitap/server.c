#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *response = "Message received";
    
    // Tạo socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Định nghĩa địa chỉ server
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Gắn địa chỉ với socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Lắng nghe kết nối từ client
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // Chấp nhận kết nối từ client
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
        perror("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Đọc dữ liệu từ client
    int read_bytes = read(new_socket, buffer, BUFFER_SIZE);
    if (read_bytes < 0) {
        perror("read failed");
        close(new_socket);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Message from client: %s\n", buffer);

    // Lấy địa chỉ của client
    char client_addr[INET_ADDRSTRLEN];
    if (inet_ntop(AF_INET, &address.sin_addr, client_addr, sizeof(client_addr)) == NULL) {
        perror("inet_ntop failed");
        close(new_socket);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client address: %s\n", client_addr);

    // Gửi phản hồi cho client
    write(new_socket, response, strlen(response));

    // Đóng kết nối
    close(new_socket);
    close(server_fd);

    return 0;
}
