#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define PORT 8080
#define MAX_CLIENTS 2
#define PUZZLE_SIZE 5

char puzzle[PUZZLE_SIZE][PUZZLE_SIZE] = {
    {'C', 'A', 'T', ' ', ' '},
    {' ', ' ', 'A', ' ', ' '},
    {'D', 'O', 'G', ' ', ' '},
    {' ', ' ', ' ', ' ', ' '},
    {' ', 'B', 'I', 'R', 'D'}
};

char answers[PUZZLE_SIZE][PUZZLE_SIZE] = {
    {'C', 'A', 'T', ' ', ' '},
    {' ', ' ', 'A', ' ', ' '},
    {'D', 'O', 'G', ' ', ' '},
    {' ', ' ', 'A', ' ', ' '},
    {' ', 'B', 'I', 'R', 'D'}
};

int isCorrectAnswer(int row, int col, char letter) {
    return answers[row][col] == letter;
}

void handleClient(int clientSock) {
    char buffer[1024];
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        recv(clientSock, buffer, sizeof(buffer), 0);

        int row, col;
        char letter;
        sscanf(buffer, "%d %d %c", &row, &col, &letter);

        if (isCorrectAnswer(row, col, letter)) {
            puzzle[row][col] = letter;
            send(clientSock, "CORRECT\n", 8, 0);
        } else {
            send(clientSock, "WRONG\n", 6, 0);
        }
    }
    close(clientSock);
}

int main() {
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSock, MAX_CLIENTS);

    printf("Waiting for players...\n");

    for (int i = 0; i < MAX_CLIENTS; i++) {
        int clientSock = accept(serverSock, NULL, NULL);
        printf("Player %d connected.\n", i + 1);
        handleClient(clientSock);
    }

    close(serverSock);
    return 0;
}

