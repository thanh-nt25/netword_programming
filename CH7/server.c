#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int game_board[3][3]; 
int move_count = 0;   

void initialize_game_board() {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            game_board[i][j] = 0;
}

void notify_turn(int client_sock) {
    unsigned char buffer[BUFFER_SIZE] = {0};
    buffer[0] = 0x05; 
    send(client_sock, buffer, BUFFER_SIZE, 0);
}

void send_state_update(int client1_sock, int client2_sock) {
    unsigned char buffer[BUFFER_SIZE] = {0};
    buffer[0] = 0x03; 
    memcpy(&buffer[1], game_board, sizeof(game_board));
    send(client1_sock, buffer, BUFFER_SIZE, 0);
    send(client2_sock, buffer, BUFFER_SIZE, 0);
}

void send_result(int client1_sock, int client2_sock, int result) {
    unsigned char buffer[BUFFER_SIZE] = {0};
    buffer[0] = 0x04;
    buffer[1] = result;
    send(client1_sock, buffer, BUFFER_SIZE, 0);
    send(client2_sock, buffer, BUFFER_SIZE, 0);
}

int check_winner() {
    for (int i = 0; i < 3; i++) {
        if (game_board[i][0] != 0 && game_board[i][0] == game_board[i][1] && game_board[i][1] == game_board[i][2])
            return game_board[i][0];
        if (game_board[0][i] != 0 && game_board[0][i] == game_board[1][i] && game_board[1][i] == game_board[2][i])
            return game_board[0][i];
    }
    if (game_board[0][0] != 0 && game_board[0][0] == game_board[1][1] && game_board[1][1] == game_board[2][2])
        return game_board[0][0];
    if (game_board[0][2] != 0 && game_board[0][2] == game_board[1][1] && game_board[1][1] == game_board[2][0])
        return game_board[0][2];
    
    return 0; 
}

void handle_move(int client_sock, int current_player, int client1_sock, int client2_sock) {
    unsigned char buffer[BUFFER_SIZE];
    recv(client_sock, buffer, BUFFER_SIZE, 0); 
    int row = buffer[1];
    int col = buffer[2];
    
    if (game_board[row][col] == 0) {
        game_board[row][col] = current_player;
        move_count++;
        send_state_update(client1_sock, client2_sock); 
        int winner = check_winner();
        
        if (winner > 0) {
            send_result(client1_sock, client2_sock, winner); 
            close(client1_sock);
            close(client2_sock);
            exit(0);
        } else if (move_count == 9) {
            send_result(client1_sock, client2_sock, 3); 
            close(client1_sock);
            close(client2_sock);
            exit(0);
        }
    }
}

int main() {
    int server_sock, client1_sock, client2_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 2);

    printf("Đang chờ kết nối từ 2 client...\n");
    
    client1_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);
    client2_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_len);

    printf("Cả 2 client đã kết nối. Bắt đầu trò chơi!\n");
    
    initialize_game_board(); 

    int current_player = 1;
    while (1) {
        if (current_player == 1) {
            notify_turn(client1_sock); 
            handle_move(client1_sock, 1, client1_sock, client2_sock);
            current_player = 2;
        } else {
            notify_turn(client2_sock); 
            handle_move(client2_sock, 2, client1_sock, client2_sock);
            current_player = 1;
        }
    }

    return 0;
}
