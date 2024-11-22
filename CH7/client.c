#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int game_board[3][3]; 

void display_game_board() {
    printf("Bảng trò chơi hiện tại:\n");
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (game_board[i][j] == 0)
                printf("- ");
            else if (game_board[i][j] == 1)
                printf("X ");
            else
                printf("O ");
        }
        printf("\n");
    }
}

void update_game_state(unsigned char* buffer) {
    memcpy(game_board, &buffer[1], sizeof(game_board));
    display_game_board();
}

int main() {
    int client_sock;
    struct sockaddr_in server_addr;
    unsigned char buffer[BUFFER_SIZE];

    //create socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8080);

    connect(client_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    printf("Đã kết nối đến server!\n");

    while (1) {
        recv(client_sock, buffer, BUFFER_SIZE, 0); 

        if (buffer[0] == 0x05) { 
            int row, col;
            printf("Nhập nước đi của bạn (hàng và cột): ");
            scanf("%d %d", &row, &col);
            buffer[0] = 0x02; // MOVE
            buffer[1] = row;
            buffer[2] = col;
            send(client_sock, buffer, BUFFER_SIZE, 0); 
        } else if (buffer[0] == 0x03) { 
            update_game_state(buffer);
        } else if (buffer[0] == 0x04) { 
            int result = buffer[1];
            if (result == 1)
                printf("Bạn đã thắng!\n");
            else if (result == 2)
                printf("Bạn đã thua!\n");
            else
                printf("Trò chơi hòa!\n");
            break;
        }
    }

    close(client_sock);
    return 0;
}
