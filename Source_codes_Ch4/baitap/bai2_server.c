#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define PORT 8080
#define MAX_CLIENTS 10
#define MAX_QUESTIONS 10
#define MAX_OPTIONS 4

typedef struct {
    char question[256];
    char options[MAX_OPTIONS][256];
    int correct_answer; // index cua dap an dung
} Question;

Question questions[MAX_QUESTIONS];

void initialize_questions() {
    strcpy(questions[0].question, "Ket qua cua phep tinh 1+1?");
    strcpy(questions[0].options[0], "1");
    strcpy(questions[0].options[1], "2");
    strcpy(questions[0].options[2], "3");
    strcpy(questions[0].options[3], "4");
    questions[0].correct_answer = 1; 

    strcpy(questions[1].question, "Ket qua cua phep tinh 2+2?");
    strcpy(questions[1].options[0], "1");
    strcpy(questions[1].options[1], "2");
    strcpy(questions[1].options[2], "3");
    strcpy(questions[1].options[3], "4");
    questions[1].correct_answer = 3; 

    strcpy(questions[2].question, "Ket qua cua phep tinh 3+3?");
    strcpy(questions[2].options[0], "6");
    strcpy(questions[2].options[1], "2");
    strcpy(questions[2].options[2], "3");
    strcpy(questions[2].options[3], "4");
    questions[2].correct_answer = 0; 

    strcpy(questions[3].question, "Ket qua cua phep tinh 4+4?");
    strcpy(questions[3].options[0], "10");
    strcpy(questions[3].options[1], "8");
    strcpy(questions[3].options[2], "3");
    strcpy(questions[3].options[3], "4");
    questions[3].correct_answer = 1; 

    strcpy(questions[4].question, "Ket qua cua phep tinh 5+5?");
    strcpy(questions[4].options[0], "10");
    strcpy(questions[4].options[1], "8");
    strcpy(questions[4].options[2], "5");
    strcpy(questions[4].options[3], "20");
    questions[4].correct_answer = 0; 

    strcpy(questions[5].question, "Ket qua cua phep tinh 6+6?");
    strcpy(questions[5].options[0], "12");
    strcpy(questions[5].options[1], "23");
    strcpy(questions[5].options[2], "31");
    strcpy(questions[5].options[3], "45");
    questions[5].correct_answer = 0; 

    strcpy(questions[6].question, "Ket qua cua phep tinh 7+7?");
    strcpy(questions[6].options[0], "10");
    strcpy(questions[6].options[1], "13");
    strcpy(questions[6].options[2], "12");
    strcpy(questions[6].options[3], "14");
    questions[6].correct_answer = 3; 

    strcpy(questions[7].question, "Ket qua cua phep tinh 8+8?");
    strcpy(questions[7].options[0], "16");
    strcpy(questions[7].options[1], "21");
    strcpy(questions[7].options[2], "34");
    strcpy(questions[7].options[3], "41");
    questions[7].correct_answer = 0; 

    strcpy(questions[8].question, "Ket qua cua phep tinh 9+9?");
    strcpy(questions[8].options[0], "15");
    strcpy(questions[8].options[1], "16");
    strcpy(questions[8].options[2], "17");
    strcpy(questions[8].options[3], "18");
    questions[8].correct_answer = 3; 

    strcpy(questions[9].question, "Ket qua cua phep tinh 10+10?");
    strcpy(questions[9].options[0], "10");
    strcpy(questions[9].options[1], "20");
    strcpy(questions[9].options[2], "30");
    strcpy(questions[9].options[3], "40");
    questions[9].correct_answer = 1; 

}

void shuffle_options(Question *q) {
    int indices[MAX_OPTIONS];
    for (int i = 0; i < MAX_OPTIONS; i++) {
        indices[i] = i;
    }

    for (int i = MAX_OPTIONS - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        // Hoán đổi chỉ số
        int temp = indices[i];
        indices[i] = indices[j];
        indices[j] = temp;
    }

    char temp_options[MAX_OPTIONS][256];
    int correct_index = -1;

    for (int i = 0; i < MAX_OPTIONS; i++) {
        strcpy(temp_options[i], q->options[indices[i]]);
        if (indices[i] == q->correct_answer) {
            correct_index = i; 
        }
    }

    for (int i = 0; i < MAX_OPTIONS; i++) {
        strcpy(q->options[i], temp_options[i]);
    }
    q->correct_answer = correct_index; 
}

void handle_client(int client_socket) {
    int score = 0;
    char buffer[1024];

    for (int i = 0; i < MAX_QUESTIONS; i++) {
        shuffle_options(&questions[i]);
        
        send(client_socket, questions[i].question, sizeof(questions[i].question), 0);
        send(client_socket, questions[i].options, sizeof(questions[i].options), 0);

        recv(client_socket, buffer, sizeof(buffer), 0);
        int answer = atoi(buffer);

        printf("Received answer: %d\n", answer);
        if (answer == questions[i].correct_answer) {
            score++; 
        }
    }

    snprintf(buffer, sizeof(buffer), "Your score: %d/10", score);
    send(client_socket, buffer, sizeof(buffer), 0);

    close(client_socket);
    exit(0); 
}

void sigchld_handler(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0); 
}

int main() {
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    initialize_questions();
    
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    
    bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
    listen(server_socket, MAX_CLIENTS);
    
    signal(SIGCHLD, sigchld_handler);
    
    printf("Server is running on port %d\n", PORT);
    
    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &addr_len);
        
        if (fork() == 0) { 
            close(server_socket); 
            handle_client(client_socket);
        }
        
        close(client_socket); 
    }
    
    return 0;
}
