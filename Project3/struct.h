#ifndef STRUCT_H
#define STRUCT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>
#include <ctype.h>

void to_upper_string(char* str) {
    for (int i = 0; i < strlen(str); i++) {
        str[i] = (char) toupper(str[i]);
    }
}

int index_of(char* str, char c) {
    for (int i = 0; i < strlen(str); i++) {
        if (str[i] == c) {
            return i;
        }
    }
    return -1;
}

#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_PORT 2209

#define CLIENTS_SIZE 22
#define GAMES_SIZE CLIENTS_SIZE
#define PAGE_SIZE 12
#define IP_LENGTH (INET_ADDRSTRLEN + 7)

#define MAX_ROW 10
#define MAX_GUESS 66

#define CROSSWORD_SIZE 78
#define SECRET_SIZE 15

#define STARTING_ELO 800

typedef enum rank {
    BEGINNER,
    INTERMEDIATE,
    ADVANCED,
    MASTER,
    GRANDMASTER,
} rank_t;

rank_t get_rank(int elo) {
    if (elo < 1200) {
        return BEGINNER;
    } else if (elo < 1600) {
        return INTERMEDIATE;
    } else if (elo < 2200) {
        return ADVANCED;
    } else if (elo < 3000) {
        return MASTER;
    } else {
        return GRANDMASTER;
    }
}

char* get_rank_str(rank_t rank) {
    switch (rank) {
        case BEGINNER:
            return "Beginner";

        case INTERMEDIATE:
            return "Intermediate";

        case ADVANCED:
            return "Advanced";

        case MASTER:
            return "Master";

        case GRANDMASTER:
            return "Grandmaster";

        default:
            return "Unknown";
    }
}

char* get_rank_str_by_elo(int elo) {
    return get_rank_str(get_rank(elo));
}

typedef struct user {
    char username[32];
    char password[32];
    int elo;
    int wins;
    int losses;
    int draws;
    int fd;
} user_t;

typedef enum guess_type {
    GUESS_ROW,
    GUESS_COL,
} guess_type_t;

typedef struct guess {
    int player;
    guess_type_t type;
    int row;
    char crossword[32];
} guess_t;

typedef struct crosswords {
    char crossword[MAX_ROW][32]; // 10 *32
    int start_cols[MAX_ROW];
    int num_rows;
    char suggestions[MAX_ROW][255];
    char secret[MAX_ROW];
    int secret_col;
} crosswords_t;

typedef enum game_status {
    G_STATUS_EMPTY,
    G_STATUS_WAITING,
    G_STATUS_RUNNING,
    G_STATUS_DRAW,
    G_STATUS_PLAYER1_WIN,
    G_STATUS_PLAYER2_WIN,
} game_status_t;

typedef struct game {
    int created;
    int client1;
    int client2;
    user_t user1;
    user_t user2;
    game_status_t status;
    crosswords_t crosswords;
    int crossword_index;
    guess_t guesses[MAX_GUESS];
    int guess_num;
    struct tm start_time;
    struct tm end_time;
    int turn;
} game_t;

typedef enum message_type {
    MSG_ERROR,
    MSG_OK,
    MSG_QUIT,
    MSG_LOGIN,
    MSG_LOGOUT,
    MSG_REGISTER,
    MSG_GET_USERS,
    MSG_CHALLENGE,
    MSG_ACCEPT,
    MSG_DECLINE,
    MSG_GUESS,
    MSG_SURRENDER,
} message_type_t;

typedef struct message {
    message_type_t type;
    user_t user;
    guess_t guess;
    game_t game;
    char message[255];
} message_t;

struct tm get_cur_time() {
    time_t t = time(NULL);
    return *localtime(&t);
}

char* get_time_str1(struct tm tm) {
    char* result = malloc(22);
    sprintf(result, "%02d:%02d:%02d %02d-%02d-%d", tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900);
    return result;
}

char* get_time_str2(struct tm tm) {
    char* result = malloc(22);
    sprintf(result, "%d%02d%02d%02d%02d%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    return result;
}

#endif // !STRUCT_H
