#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "struct.h"

const char* USERS_DATA_FILE = "users.dat";
const user_t EMPTY_USER = {"", "", 0, 0, 0, 0};

#define CROSSWORDS_SIZE 2
crosswords_t crosswords[CROSSWORDS_SIZE];

int server_socket;

int client_fds[CLIENTS_SIZE];
char ips[CLIENTS_SIZE][IP_LENGTH];
int num_clients = 0;

user_t users[CLIENTS_SIZE];
int num_users = 0;

game_t games[GAMES_SIZE];

int insert_user(user_t* user);
user_t* get_user(char* username);
int update_user(user_t* user);

int get_max_fd();
int add_client();
void remove_client(int client_fd);
char* get_ip(int client_fd);

user_t get_user_by_fd(int client_fd);
user_t get_user_by_username(char* username);
void login_user(user_t* user);
void logout_user(user_t* user);
int is_logged_in(user_t* user);

void init();
void init_crosswords();

void clear_game(game_t* game);
void exit_game(game_t* game, int client_fd);
void start_game(game_t* game);
void finish_game(game_t* game, int client_fd);

void handle_disconnected(int client_fd);
void handle_register(int client_fd, message_t* request);
void handle_login(int client_fd, message_t* request);
void handle_logout(int client_fd, message_t* request);
void handle_get_users(int client_fd, message_t* request);
void handle_challenge(int client_fd, message_t* request);
void handle_accept(int client_fd, message_t* request);
void handle_decline(int client_fd, message_t* request);
void handle_guess(int client_fd, message_t* request);
void handle_surrender(int client_fd, message_t* request);

int main(int argc, char* argv[]) {
    srandom(time(NULL));

    uint16_t port = DEFAULT_PORT;
    if (argc > 1) {
        port = strtol(argv[1], NULL, 10);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) < 0) {
        perror("Cannot bind socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 22) < 0) {
        perror("Cannot listen to socket");
        exit(EXIT_FAILURE);
    }

    init();
    init_crosswords();

    printf("> Server is running on port %d\n", port);

    fd_set read_fds;
    message_t request;

    while (1) {
        FD_ZERO(&read_fds);
        FD_SET(server_socket, &read_fds);
        for (int i = 0; i < num_clients; i++) {
            FD_SET(client_fds[i], &read_fds);
        }

        select(get_max_fd(), &read_fds, NULL, NULL, NULL);
        if (FD_ISSET(server_socket, &read_fds)) {
            if (add_client() == 0) {
                printf("> Cannot add more clients\n");
                continue;
            }
        }

        for (int i = 0; i < num_clients; i++) {
            if (FD_ISSET(client_fds[i], &read_fds)) {
                memset(&request, 0x00, sizeof(request));
                ssize_t received_len = recv(client_fds[i], &request, sizeof(request), 0);
                if (received_len <= 0) {
                    handle_disconnected(client_fds[i]);
                    continue;
                }

                switch (request.type) {
                    case MSG_QUIT:
                        handle_disconnected(client_fds[i]);
                        break;

                    case MSG_REGISTER:
                        handle_register(client_fds[i], &request);
                        break;

                    case MSG_LOGIN:
                        handle_login(client_fds[i], &request);
                        break;

                    case MSG_LOGOUT:
                        handle_logout(client_fds[i], &request);
                        break;

                    case MSG_GET_USERS:
                        handle_get_users(client_fds[i], &request);
                        break;

                    case MSG_CHALLENGE:
                        handle_challenge(client_fds[i], &request);
                        break;

                    case MSG_ACCEPT:
                        handle_accept(client_fds[i], &request);
                        break;

                    case MSG_DECLINE:
                        handle_decline(client_fds[i], &request);
                        break;

                    case MSG_GUESS:
                        handle_guess(client_fds[i], &request);
                        break;

                    case MSG_SURRENDER:
                        handle_surrender(client_fds[i], &request);
                        break;

                    default:
                        printf("%d> Invalid request(%d)\n", client_fds[i], request.type);
                        break;
                }
            }
        }
    }

    return 0;
}

int insert_user(user_t* user) {
    int fd = open(USERS_DATA_FILE, O_CREAT | O_RDWR, 0666);
    if (fd < 0) {
        perror("> Cannot open USERS_DATA_FILE");
        return 0;
    }

    lseek(fd, 0, SEEK_END);
    if (write(fd, user, sizeof(user_t)) < 0) {
        perror("> Cannot write to USERS_DATA_FILE");
        close(fd);
        return 0;
    }

    close(fd);
    return 1;
}

user_t* get_user(char* username) {
    int fd = open(USERS_DATA_FILE, O_RDONLY);
    if (fd < 0) {
        perror("> Cannot open USERS_DATA_FILE");
        return NULL;
    }

    user_t* user = (user_t*) malloc(sizeof(user_t));
    while (read(fd, user, sizeof(user_t)) > 0) {
        if (strcmp(user->username, username) == 0) {
            close(fd);
            return user;
        }
    }

    close(fd);
    free(user);
    return NULL;
}

int update_user(user_t* user) {
    int fd = open(USERS_DATA_FILE, O_RDWR);
    if (fd < 0) {
        perror("> Cannot open USERS_FILE");
        return 0;
    }

    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, user->username) == 0) {
            users[i].elo = user->elo;
            users[i].wins = user->wins;
            users[i].losses = user->losses;
            users[i].draws = user->draws;
            break;
        }
    }

    user_t* temp = (user_t*) malloc(sizeof(user_t));
    while (read(fd, temp, sizeof(user_t)) > 0) {
        if (strcmp(temp->username, user->username) == 0) {
            lseek(fd, (off_t) -sizeof(user_t), SEEK_CUR);
            temp->elo = user->elo;
            temp->wins = user->wins;
            temp->losses = user->losses;
            temp->draws = user->draws;
            if (write(fd, temp, sizeof(user_t)) < 0) {
                perror("> Cannot write to USERS_FILE");
                close(fd);
                free(temp);
                return 0;
            }

            close(fd);
            free(temp);
            return 1;
        }
    }

    close(fd);
    free(temp);
    return 1;
}

int get_max_fd() {
    int max = server_socket;
    for (int i = 0; i < num_clients; i++) {
        if (max < client_fds[i]) {
            max = client_fds[i];
        }
    }
    return max + 1;
}

int add_client() {
    if (num_clients >= CLIENTS_SIZE) {
        return 0;
    }

    struct sockaddr_in client_address;
    socklen_t client_address_length = sizeof(client_address);

    int client_fd = accept(server_socket, (struct sockaddr*) &client_address, &client_address_length);
    client_fds[num_clients] = client_fd;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    socklen_t addr_size = sizeof(struct sockaddr_in);
    getpeername(client_fd, (struct sockaddr*) &addr, &addr_size);

    char ipv4[IP_LENGTH];
    strcpy(ipv4, inet_ntoa(addr.sin_addr));

    char port[7];
    sprintf(port, ":%hu", ntohs(addr.sin_port));
    strcat(ipv4, port);

    strcpy(ips[num_clients], ipv4);

    num_clients++;

    printf("> ADD_CLIENT(): %d -> Total clients: %d %s\n", client_fd, num_clients, ipv4);
    return 1;
}

void remove_client(int client_fd) {
    int index = -1;
    for (int i = 0; i < num_clients; i++) {
        if (client_fds[i] == client_fd) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        return;
    }

    if (index != num_clients - 1) {
        client_fds[index] = client_fds[num_clients - 1];
        strcpy(ips[index], ips[num_clients - 1]);
    }

    client_fds[num_clients - 1] = -1;
    memset(ips[num_clients - 1], 0x00, IP_LENGTH);
    close(client_fd);

    num_clients--;
    printf("> REMOVE_CLIENT(): %d -> Totals clients: %d\n", client_fd, num_clients);
}

char* get_ip(int client_fd) {
    for (int i = 0; i < num_clients; i++) {
        if (client_fds[i] == client_fd) {
            return ips[i];
        }
    }
    return NULL;
}

user_t get_user_by_fd(int client_fd) {
    for (int i = 0; i < num_users; i++) {
        if (users[i].fd == client_fd) {
            return users[i];
        }
    }
    return EMPTY_USER;
}

user_t get_user_by_username(char* username) {
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, username) == 0) {
            return users[i];
        }
    }
    return EMPTY_USER;
}

void login_user(user_t* user) {
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, user->username) == 0) {
            return;
        }
    }

    users[num_users] = *user;
    num_users++;
    printf("> LOGIN(): %s(%d)\n", user->username, user->fd);
}

void logout_user(user_t* user) {
    int index = -1;
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, user->username) == 0) {
            index = i;
            break;
        }
    }

    if (index == -1 || index == num_users) {
        return;
    }

    if (index != num_users - 1) {
        users[index] = users[num_users - 1];
    }

    memset(&users[num_users - 1], 0x00, sizeof(user_t));
    num_users--;
    printf("> LOGOUT(): %s\n", user->username);
}

int is_logged_in(user_t* user) {
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, user->username) == 0) {
            return 1;
        }
    }
    return 0;
}

void init() {
    for (int i = 0; i < GAMES_SIZE; i++) {
        clear_game(&games[i]);
    }
}

void init_crosswords() {
    crosswords_t crosswords1;
    crosswords1.num_rows = 9;

    crosswords1.start_cols[0] = 1;
    strcpy(crosswords1.crossword[0], "BENNHARONG");
    strcpy(crosswords1.suggestions[0], "Ngay 5/6/1911, tu ben cang nao Nguyen Tat Thanh ra di tim duong cuu nuoc?");

    crosswords1.start_cols[1] = 0;
    strcpy(crosswords1.crossword[1], "LIENXO");
    strcpy(crosswords1.suggestions[1], "Tu nam 1923 den nam 1924, Nguyen Ai Quoc hoat dong tai?");

    crosswords1.start_cols[2] = 0;
    strcpy(crosswords1.crossword[2], "QUANGCHAU");
    strcpy(crosswords1.suggestions[2], "Cuoi nam 1924, Nguyen Ai Quoc tu Lien Xo ve?");

    crosswords1.start_cols[3] = 0;
    strcpy(crosswords1.crossword[3], "CONGNHAN");
    strcpy(crosswords1.suggestions[3], "Giai cap nao bi ba tang ap buc boc lot cua thuc dan, phong kien, tu san nguoi Viet, co quan he tu nhien gan bo voi giai cap nong dan?");

    crosswords1.start_cols[4] = 3;
    strcpy(crosswords1.crossword[4], "LOIKEUGOI");
    strcpy(crosswords1.suggestions[4], "Nhan dip thanh lap Dang Cong san Viet Nam, Nguyen Ai Quoc da ra?");

    crosswords1.start_cols[5] = 1;
    strcpy(crosswords1.crossword[5], "ANNAMCONGSANDANG");
    strcpy(crosswords1.suggestions[5], "Thang 8/1929, to chuc cong san nao da ra doi?");

    crosswords1.start_cols[6] = 4;
    strcpy(crosswords1.crossword[6], "VIETNAMQUOCDANDANG");
    strcpy(crosswords1.suggestions[6], "Dang chinh tri theo xu huong cach mang dan chu tu san, tieu bieu cho bo phan tu san dan toc Viet Nam, ra doi ngay 25/12/1927 la?");

    crosswords1.start_cols[7] = 1;
    strcpy(crosswords1.crossword[7], "BASON");
    strcpy(crosswords1.suggestions[7], "Cuoc bai cong nao da danh dau buoc tien moi cua phong trao cong nhan Viet Nam - giai cap cong nhan Viet Nam tu day buoc dau di vao dau tranh co to chuc va muc dich chinh tri ro rang?");

    crosswords1.start_cols[8] = 4;
    strcpy(crosswords1.crossword[8], "KHAIMO");
    strcpy(crosswords1.suggestions[8], "Trong chuong trinh khai thac thuoc dia lan thu hai, Phap tang cuong dau tu vao Viet Nam, bo von nhieu nhat vao nong nghiep (chu yeu la don dien cao su) va?");

    strcpy(crosswords1.secret, "HOCHIMINH");
    crosswords1.secret_col = 5;

    crosswords_t crosswords2;
    crosswords2.num_rows = 9;

    crosswords2.start_cols[0] = 1;
    strcpy(crosswords2.crossword[0], "BENNHARONG");
    strcpy(crosswords2.suggestions[0], "Ngay 5/6/1911, tu ben cang nao Nguyen Tat Thanh ra di tim duong cuu nuoc?");

    crosswords2.start_cols[1] = 0;
    strcpy(crosswords2.crossword[1], "LIENXO");
    strcpy(crosswords2.suggestions[1], "Tu nam 1923 den nam 1924, Nguyen Ai Quoc hoat dong tai?");

    crosswords2.start_cols[2] = 0;
    strcpy(crosswords2.crossword[2], "QUANGCHAU");
    strcpy(crosswords2.suggestions[2], "Cuoi nam 1924, Nguyen Ai Quoc tu Lien Xo ve?");

    crosswords2.start_cols[3] = 0;
    strcpy(crosswords2.crossword[3], "CONGNHAN");
    strcpy(crosswords2.suggestions[3], "Giai cap nao bi ba tang ap buc boc lot cua thuc dan, phong kien, tu san nguoi Viet, co quan he tu nhien gan bo voi giai cap nong dan?");

    crosswords2.start_cols[4] = 3;
    strcpy(crosswords2.crossword[4], "LOIKEUGOI");
    strcpy(crosswords2.suggestions[4], "Nhan dip thanh lap Dang Cong san Viet Nam, Nguyen Ai Quoc da ra?");

    crosswords2.start_cols[5] = 1;
    strcpy(crosswords2.crossword[5], "ANNAMCONGSANDANG");
    strcpy(crosswords2.suggestions[5], "Thang 8/1929, to chuc cong san nao da ra doi?");

    crosswords2.start_cols[6] = 4;
    strcpy(crosswords2.crossword[6], "VIETNAMQUOCDANDANG");
    strcpy(crosswords2.suggestions[6], "Dang chinh tri theo xu huong cach mang dan chu tu san, tieu bieu cho bo phan tu san dan toc Viet Nam, ra doi ngay 25/12/1927 la?");

    crosswords2.start_cols[7] = 1;
    strcpy(crosswords2.crossword[7], "BASON");
    strcpy(crosswords2.suggestions[7], "Cuoc bai cong nao da danh dau buoc tien moi cua phong trao cong nhan Viet Nam - giai cap cong nhan Viet Nam tu day buoc dau di vao dau tranh co to chuc va muc dich chinh tri ro rang?");

    crosswords2.start_cols[8] = 4;
    strcpy(crosswords2.crossword[8], "KHAIMO");
    strcpy(crosswords2.suggestions[8], "Trong chuong trinh khai thac thuoc dia lan thu hai, Phap tang cuong dau tu vao Viet Nam, bo von nhieu nhat vao nong nghiep (chu yeu la don dien cao su) va?");

    strcpy(crosswords2.secret, "HOCHIMINH");
    crosswords2.secret_col = 5;

    crosswords[0] = crosswords1;
    crosswords[1] = crosswords2;
}

void clear_game(game_t* game) {
    game->client1 = -1;
    game->client2 = -1;
    game->user1 = EMPTY_USER;
    game->user2 = EMPTY_USER;
    game->status = G_STATUS_EMPTY;
    game->turn = -1;

    for (int i = 0; i < 22; i++) {
        memset(game->crosswords.crossword[i], 0x00, 22);
        memset(game->crosswords.suggestions[i], 0x00, 255);
    }
    game->crosswords.num_rows = 0;
    memset(game->crosswords.secret, 0x00, 32);
    game->crosswords.secret_col = -1;
    game->crossword_index = -1;
}

void exit_game(game_t* game, int client_fd) {
    if (game->client1 == client_fd && game->client2 != -1) {
        if (game->status == G_STATUS_WAITING && game->created != client_fd) {
            message_t response;
            response.type = MSG_DECLINE;
            strcpy(response.message, "Opponent declined the challenge");
            send(game->client2, &response, sizeof(response), 0);
            printf("%d> EXIT_GAME(): 1 %s\n", client_fd, response.message);
        } else if (game->status == G_STATUS_RUNNING) {
            message_t response;
            response.type = MSG_SURRENDER;
            strcpy(response.message, "Opponent surrendered");
            user_t* winner = get_user(game->user2.username);
            user_t* loser = get_user(game->user1.username);
            winner->wins++;
            winner->elo += 5;
            loser->losses++;
            update_user(winner);
            update_user(loser);

            for (int i = 0; i < game->crosswords.num_rows; i++) {
                strcpy(game->crosswords.crossword[i], crosswords[game->crossword_index].crossword[i]);
            }

            response.user = *winner;
            response.game = *game;
            send(game->client2, &response, sizeof(response), 0);
            printf("%d> EXIT_GAME(): 2 %s\n", client_fd, response.message);
        }
    } else if (game->client2 == client_fd && game->client1 != -1) {
        if (game->status == G_STATUS_WAITING && game->created != client_fd) {
            message_t response;
            response.type = MSG_DECLINE;
            strcpy(response.message, "Opponent declined the challenge");
            send(game->client1, &response, sizeof(response), 0);
            printf("%d> EXIT_GAME(): 3 %s\n", client_fd, response.message);
        } else if (game->status == G_STATUS_RUNNING) {
            message_t response;
            response.type = MSG_SURRENDER;
            strcpy(response.message, "Opponent surrendered");
            user_t* winner = get_user(game->user1.username);
            user_t* loser = get_user(game->user2.username);
            winner->wins++;
            winner->elo += 5;
            loser->losses++;
            update_user(winner);
            update_user(loser);

            for (int i = 0; i < game->crosswords.num_rows; i++) {
                strcpy(game->crosswords.crossword[i], crosswords[game->crossword_index].crossword[i]);
            }

            response.user = *winner;
            response.game = *game;
            send(game->client1, &response, sizeof(response), 0);
            printf("%d> EXIT_GAME(): 4 %s\n", client_fd, response.message);
        }
    }

    clear_game(game);
}

void start_game(game_t* game) {
    game->status = G_STATUS_RUNNING;
    game->start_time = get_cur_time();
    game->turn = random() % 2;
    game->crossword_index = random() % CROSSWORDS_SIZE;
    game->crosswords = crosswords[game->crossword_index];
    game->guess_num = 0;

    for (int i = 0; i < 22; i++) {
        memset(game->crosswords.crossword[i], 0x00, 32);
        for (int j = 0; j < strlen(crosswords[game->crossword_index].crossword[i]); j++) {
            game->crosswords.crossword[i][j] = ' ';
        }
    }
    memset(game->crosswords.secret, 0x00, 32);
}

void finish_game(game_t* game, int client_fd) {
    game->status = G_STATUS_DRAW;
    game->end_time = get_cur_time();

    if (game->client1 == client_fd) {
        game->status = G_STATUS_PLAYER1_WIN;

        user_t* winner = get_user(game->user1.username);
        user_t* loser = get_user(game->user2.username);
        winner->wins++;
        winner->elo += 5;
        loser->losses++;
        update_user(winner);
        update_user(loser);

        printf("%d> FINISH_GAME(): %s wins\n", client_fd, game->user1.username);
    } else if (game->client2 == client_fd) {
        game->status = G_STATUS_PLAYER2_WIN;

        user_t* winner = get_user(game->user2.username);
        user_t* loser = get_user(game->user1.username);
        winner->wins++;
        winner->elo += 5;
        loser->losses++;
        update_user(winner);
        update_user(loser);

        printf("%d> FINISH_GAME(): %s wins\n", client_fd, game->user2.username);
    } else {
        printf("%d> FINISH_GAME(): Draw\n", client_fd);
    }
}

void handle_disconnected(int client_fd) {
    char* ip = get_ip(client_fd);
    printf("%d> DISCONNECTED: %s\n", client_fd, ip);
    remove_client(client_fd);
    user_t user = get_user_by_fd(client_fd);
    logout_user(&user);

    for (int game_id = 0; game_id < GAMES_SIZE; game_id++) {
        if (games[game_id].client1 == client_fd || games[game_id].client2 == client_fd) {
            exit_game(&games[game_id], client_fd);
            break;
        }
    }
}

void handle_register(int client_fd, message_t* request) {
    message_t response;
    response.type = MSG_ERROR;
    user_t user = request->user;

    if (strlen(user.username) < 3 || strlen(user.username) > 30) {
        strcpy(response.message, "Invalid username");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> REGISTER(): %s\n", client_fd, response.message);
        return;
    }

    if (strlen(user.password) < 3 || strlen(user.password) > 30) {
        strcpy(response.message, "Invalid password");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> REGISTER(): %s\n", client_fd, response.message);
        return;
    }

    user_t* result = get_user(user.username);
    if (result != NULL) {
        strcpy(response.message, "Username already exists");
        send(client_fd, &response, sizeof(response), 0);
        free(result);
        printf("%d> REGISTER(): %s %s\n", client_fd, response.message, user.username);
        return;
    }

    user.elo = STARTING_ELO;
    user.wins = 0;
    user.losses = 0;
    user.draws = 0;
    if (insert_user(&user) == 0) {
        strcpy(response.message, "An error occurred");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> REGISTER(): %s %s\n", client_fd, response.message, user.username);
        return;
    }

    response.type = MSG_OK;
    strcpy(response.message, "Register successfully");
    send(client_fd, &response, sizeof(response), 0);
    printf("%d> REGISTER(): %s\n", client_fd, response.message);
}

void handle_login(int client_fd, message_t* request) {
    message_t response;
    response.type = MSG_ERROR;

    if (strlen(request->user.username) < 3 || strlen(request->user.username) > 30) {
        strcpy(response.message, "Invalid username");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> LOGIN(): %s\n", client_fd, response.message);
        return;
    }

    if (strlen(request->user.password) < 3 || strlen(request->user.password) > 30) {
        strcpy(response.message, "Invalid password");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> LOGIN(): %s\n", client_fd, response.message);
        return;
    }

    user_t* user = get_user(request->user.username);
    if (user == NULL) {
        strcpy(response.message, "User does not exist");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> LOGIN(): %s\n", client_fd, response.message);
        return;
    }

    if (strcmp(user->password, request->user.password) != 0) {
        strcpy(response.message, "Invalid password");
        send(client_fd, &response, sizeof(response), 0);
        free(user);
        printf("%d> LOGIN(): %s\n", client_fd, response.message);
        return;
    }

    if (is_logged_in(user)) {
        strcpy(response.message, "User is already logged in");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> LOGIN(): %s %s\n", client_fd, response.message, user->username);
        free(user);
        return;
    }

    strcpy(response.user.username, user->username);
    strcpy(response.user.password, user->password);
    response.user.elo = user->elo;
    response.user.wins = user->wins;
    response.user.losses = user->losses;

    user->fd = client_fd;
    login_user(user);

    response.type = MSG_OK;
    strcpy(response.message, "Login successfully");
    send(client_fd, &response, sizeof(response), 0);
    printf("%d> LOGIN(): %s\n", client_fd, response.message);
    free(user);
}

void handle_logout(int client_fd, message_t* request) {
    message_t response;
    response.type = MSG_ERROR;
    user_t user = get_user_by_fd(client_fd);

    if (strcmp(user.username, "") == 0) {
        strcpy(response.message, "You are not logged in");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> LOGOUT(): %s\n", client_fd, response.message);
        return;
    }

    if (strcmp(user.username, request->user.username) != 0) {
        strcpy(response.message, "Invalid request");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> LOGOUT(): %s\n", client_fd, response.message);
        return;
    }

    logout_user(&user);

    for (int game_id = 0; game_id < GAMES_SIZE; game_id++) {
        if (games[game_id].client1 == client_fd || games[game_id].client2 == client_fd) {
            exit_game(&games[game_id], client_fd);
            break;
        }
    }

    response.type = MSG_OK;
    strcpy(response.message, "Logout successfully");
    send(client_fd, &response, sizeof(response), 0);
    printf("%d> LOGOUT(): %s %s\n", client_fd, response.message, request->user.username);
}

void handle_get_users(int client_fd, message_t* request) {
    message_t response;
    response.type = MSG_ERROR;
    user_t user = get_user_by_fd(client_fd);

    if (strcmp(user.username, "") == 0) {
        strcpy(response.message, "You are not logged in");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> GET_USERS(): %s\n", client_fd, response.message);
        return;
    }

    if (strcmp(user.username, request->user.username) != 0) {
        strcpy(response.message, "Invalid request");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> GET_USERS(): %s\n", client_fd, response.message);
        return;
    }

    response.type = MSG_OK;
    strcpy(response.message, "Get users successfully");
    send(client_fd, &response, sizeof(response), 0);

    int users_size = 0;
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, user.username) == 0) {
            continue;
        }
        users_size++;
    }
    send(client_fd, &users_size, sizeof(int), 0);
    for (int i = 0; i < num_users; i++) {
        if (strcmp(users[i].username, user.username) == 0) {
            continue;
        }
        printf("%d> GET_USERS(): %s %s\n", client_fd, user.username, users[i].username);
        send(client_fd, &users[i], sizeof(user_t), 0);
    }

    printf("%d> GET_USERS(): %s %d\n", client_fd, response.message, users_size);
}

void handle_challenge(int client_fd, message_t* request) {
    message_t response;
    response.type = MSG_ERROR;
    user_t user = get_user_by_fd(client_fd);

    if (strcmp(user.username, "") == 0) {
        strcpy(response.message, "You are not logged in");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> GET_USERS(): %s\n", client_fd, response.message);
        return;
    }

    user_t opponent = get_user_by_username(request->user.username);
    if (strcmp(opponent.username, "") == 0) {
        strcpy(response.message, "User does not online");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> CHALLENGE(): %s\n", client_fd, response.message);
        return;
    }

    if (strcmp(user.username, opponent.username) == 0) {
        strcpy(response.message, "You cannot challenge yourself");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> CHALLENGE(): %s\n", client_fd, response.message);
        return;
    }

    for (int i = 0; i < GAMES_SIZE; i++) {
        if (games[i].client1 == client_fd || games[i].client2 == client_fd) {
            strcpy(response.message, "You are already in a game");
            send(client_fd, &response, sizeof(response), 0);
            printf("%d> CHALLENGE(): %s\n", client_fd, response.message);
            return;
        }

        if (games[i].client1 == opponent.fd || games[i].client2 == opponent.fd) {
            strcpy(response.message, "Opponent is already in a game");
            send(client_fd, &response, sizeof(response), 0);
            printf("%d> CHALLENGE(): %s\n", client_fd, response.message);
            return;
        }
    }

    int is_created = 0;
    for (int i = 0; i < GAMES_SIZE; i++) {
        if (games[i].status == G_STATUS_EMPTY) {
            games[i].created = client_fd;
            games[i].client1 = client_fd;
            games[i].client2 = opponent.fd;
            games[i].user1 = user;
            games[i].user2 = opponent;
            games[i].status = G_STATUS_WAITING;
            is_created = 1;
            printf("%d> CHALLENGE(): created(%d)\n", client_fd, games[i].created);
            printf("%d> CHALLENGE(): client1(%d)\n", client_fd, games[i].client1);
            printf("%d> CHALLENGE(): client2(%d)\n", client_fd, games[i].client2);
            printf("%d> CHALLENGE(): users1(%s)\n", client_fd, games[i].user1.username);
            printf("%d> CHALLENGE(): users2(%s)\n", client_fd, games[i].user2.username);
            break;
        }
    }

    if (is_created == 0) {
        strcpy(response.message, "Cannot create a new game");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> CHALLENGE(): %s\n", client_fd, response.message);
        return;
    }

    response.type = MSG_CHALLENGE;
    response.user = user;
    send(opponent.fd, &response, sizeof(response), 0);
    printf("%d> CHALLENGE(): %s %s\n", client_fd, user.username, opponent.username);

    response.type = MSG_OK;
    strcpy(response.message, "Challenge sent");
    send(client_fd, &response, sizeof(response), 0);

    printf("%d> CHALLENGE(): %s\n", client_fd, response.message);
}

void handle_accept(int client_fd, message_t* request) {
    message_t response;
    response.type = MSG_ERROR;
    user_t user = get_user_by_fd(client_fd);

    if (strcmp(user.username, "") == 0) {
        strcpy(response.message, "You are not logged in");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> GET_USERS(): %s\n", client_fd, response.message);
        return;
    }

    user_t opponent = get_user_by_username(request->user.username);
    if (strcmp(opponent.username, "") == 0) {
        strcpy(response.message, "User does not online");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> ACCEPT(): %s\n", client_fd, response.message);
        return;
    }

    if (strcmp(user.username, opponent.username) == 0) {
        strcpy(response.message, "You cannot response to yourself");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> CHALLENGE(): %s\n", client_fd, response.message);
        return;
    }

    game_t* game = NULL;
    for (int i = 0; i < GAMES_SIZE; i++) {
        if (games[i].client1 == client_fd && games[i].client2 == opponent.fd) {
            game = &games[i];
            break;
        } else if (games[i].client1 == opponent.fd && games[i].client2 == client_fd) {
            game = &games[i];
            break;
        }
    }

    if (game == NULL) {
        strcpy(response.message, "Game does not exist");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> ACCEPT(): %s\n", client_fd, response.message);
        return;
    }

    if (game->status != G_STATUS_WAITING) {
        strcpy(response.message, "Game is not waiting");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> ACCEPT(): %s\n", client_fd, response.message);
        return;
    }

    start_game(game);
    game->crosswords.secret_col = crosswords[game->crossword_index].secret_col;

    printf("%d> ACCEPT(): %s %s\n", client_fd, game->user1.username, game->user2.username);
    for (int i = 0; i < game->crosswords.num_rows; i++) {
        printf("%d> ACCEPT(): %d > %s(%ld)\n", client_fd, game->crosswords.start_cols[i], game->crosswords.crossword[i], strlen(game->crosswords.crossword[i]));
    }
    printf("%d> ACCEPT(): secret_col(%d, %d)\n", client_fd, game->crosswords.secret_col, crosswords[game->crossword_index].secret_col);

    response.type = MSG_ACCEPT;
    response.user = user;
    response.game = *game;
    send(opponent.fd, &response, sizeof(response), 0);
    printf("%d> ACCEPT(): %s %s\n", client_fd, user.username, opponent.username);

    response.type = MSG_OK;
    response.user = opponent;
    strcpy(response.message, "Challenge accepted");
    send(client_fd, &response, sizeof(response), 0);
    printf("%d> ACCEPT(): %s\n", client_fd, response.message);
}

void handle_decline(int client_fd, message_t* request) {
    message_t response;
    response.type = MSG_ERROR;
    user_t user = get_user_by_fd(client_fd);

    if (strcmp(user.username, "") == 0) {
        strcpy(response.message, "You are not logged in");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> DECLINE(): %s\n", client_fd, response.message);
        return;
    }

    user_t opponent = get_user_by_username(request->user.username);
    if (strcmp(opponent.username, "") == 0) {
        strcpy(response.message, "User does not online");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> DECLINE(): %s\n", client_fd, response.message);
        return;
    }

    if (strcmp(user.username, opponent.username) == 0) {
        strcpy(response.message, "You cannot response to yourself");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> DECLINE(): %s\n", client_fd, response.message);
        return;
    }

    game_t* game = NULL;
    for (int i = 0; i < GAMES_SIZE; i++) {
        if (games[i].client1 == client_fd && games[i].client2 == opponent.fd) {
            game = &games[i];
            break;
        } else if (games[i].client1 == opponent.fd && games[i].client2 == client_fd) {
            game = &games[i];
            break;
        }
    }

    if (game == NULL) {
        strcpy(response.message, "Game does not exist");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> DECLINE(): %s\n", client_fd, response.message);
        return;
    }

    if (game->status != G_STATUS_WAITING) {
        strcpy(response.message, "Game is not waiting");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> DECLINE(): %s\n", client_fd, response.message);
        return;
    }

    exit_game(game, client_fd);

    response.type = MSG_OK;
    strcpy(response.message, "Challenge declined");
    send(client_fd, &response, sizeof(response), 0);
    printf("%d> DECLINE(): %s\n", client_fd, response.message);
}

void handle_guess(int client_fd, message_t* request) {
    message_t response;
    response.type = MSG_ERROR;
    user_t user = get_user_by_fd(client_fd);

    if (strcmp(user.username, "") == 0) {
        strcpy(response.message, "You are not logged in");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> GUESS(): %s\n", client_fd, response.message);
        return;
    }

    game_t* game = NULL;
    for (int i = 0; i < GAMES_SIZE; i++) {
        if (games[i].client1 == client_fd || games[i].client2 == client_fd) {
            game = &games[i];
            break;
        }
    }

    if (game == NULL) {
        strcpy(response.message, "You are not in a game");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> GUESS(): %s\n", client_fd, response.message);
        return;
    }

    if (game->status != G_STATUS_RUNNING) {
        strcpy(response.message, "Game is not running");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> GUESS(): %s\n", client_fd, response.message);
        return;
    }

    if (game->turn == 0 && game->client1 != client_fd) {
        strcpy(response.message, "It is not your turn");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> GUESS(): %s\n", client_fd, response.message);
        return;
    }

    if (game->turn == 1 && game->client2 != client_fd) {
        strcpy(response.message, "It is not your turn");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> GUESS(): %s\n", client_fd, response.message);
        return;
    }

    if (request->guess.type == GUESS_ROW) {
        if (request->guess.row < 0 || request->guess.row >= game->crosswords.num_rows) {
            strcpy(response.message, "Invalid row");
            send(client_fd, &response, sizeof(response), 0);
            printf("%d> GUESS(): %s\n", client_fd, response.message);
            return;
        }

        if (game->crosswords.crossword[request->guess.row][0] != ' ') {
            strcpy(response.message, "Row has been guessed");
            send(client_fd, &response, sizeof(response), 0);
            printf("%d> GUESS(): %s\n", client_fd, response.message);
            return;
        }

        to_upper_string(request->guess.crossword);
        request->guess.player = game->turn;
        game->guesses[game->guess_num] = request->guess;
        game->guess_num++;
        if (strcmp(request->guess.crossword, crosswords[game->crossword_index].crossword[request->guess.row]) != 0) {
            strcpy(response.message, "Incorrect guess");
            game->turn = 1 - game->turn;
            response.game = *game;
            send(client_fd, &response, sizeof(response), 0);

            response.type = MSG_OK;
            send(game->client1 + game->client2 - client_fd, &response, sizeof(response), 0);
            printf("%d> GUESS(): %s\n", client_fd, response.message);
            return;
        }

        response.type = MSG_OK;
        strcpy(response.message, "Correct guess");
        strcpy(game->crosswords.crossword[request->guess.row], request->guess.crossword);
        response.game = *game;
        send(client_fd, &response, sizeof(response), 0);
        send(game->client1 + game->client2 - client_fd, &response, sizeof(response), 0);

        printf("%d> GUESS(): %s\n", client_fd, response.message);
    } else if (request->guess.type == GUESS_COL) {
        to_upper_string(request->guess.crossword);

        if (strcmp(request->guess.crossword, crosswords[game->crossword_index].secret) != 0) {
            strcpy(response.message, "Incorrect guess");
            game->turn = 1 - game->turn;
            response.game = *game;
            send(client_fd, &response, sizeof(response), 0);

            response.type = MSG_OK;
            send(game->client1 + game->client2 - client_fd, &response, sizeof(response), 0);
            printf("%d> GUESS(): %s\n", client_fd, response.message);
            return;
        }

        response.type = MSG_OK;
        strcpy(response.message, "Correct guess");
        strcpy(game->crosswords.secret, request->guess.crossword);
        for (int i = 0; i < game->crosswords.num_rows; i++) {
            strcpy(game->crosswords.crossword[i], crosswords[game->crossword_index].crossword[i]);
        }

        game->guesses[game->guess_num] = request->guess;
        game->guess_num++;

        finish_game(game, client_fd);
        response.game = *game;

        if (client_fd == game->client1) {
            response.user = game->user1;
        } else {
            response.user = game->user2;
        }
        strcpy(response.message, "You win");
        send(client_fd, &response, sizeof(response), 0);

        if (client_fd == game->client1) {
            response.user = game->user2;
        } else {
            response.user = game->user1;
        }
        strcpy(response.message, "You lose");
        send(game->client1 + game->client2 - client_fd, &response, sizeof(response), 0);

        printf("%d> GUESS(): %s\n", client_fd, response.message);

        clear_game(game);
    }
}

void handle_surrender(int client_fd, message_t* request) {
    message_t response;
    response.type = MSG_ERROR;
    user_t user = get_user_by_fd(client_fd);

    if (strcmp(user.username, "") == 0) {
        strcpy(response.message, "You are not logged in");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> SURRENDER(): %s\n", client_fd, response.message);
        return;
    }

    game_t* game = NULL;
    for (int i = 0; i < GAMES_SIZE; i++) {
        if (games[i].client1 == client_fd || games[i].client2 == client_fd) {
            game = &games[i];
            break;
        }
    }

    if (game == NULL) {
        strcpy(response.message, "You are not in a game");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> SURRENDER(): %s\n", client_fd, response.message);
        return;
    }

    if (game->status != G_STATUS_RUNNING) {
        strcpy(response.message, "Game is not running");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> SURRENDER(): %s\n", client_fd, response.message);
        return;
    }

    if (game->client1 == client_fd) {
        user_t* winner = get_user(game->user2.username);
        user_t* loser = get_user(game->user1.username);
        winner->wins++;
        winner->elo += 5;
        loser->losses++;
        update_user(winner);
        update_user(loser);
        response.user = *winner;
        game->status = G_STATUS_PLAYER2_WIN;

        for (int i = 0; i < game->crosswords.num_rows; i++) {
            strcpy(game->crosswords.crossword[i], crosswords[game->crossword_index].crossword[i]);
        }

        response.type = MSG_OK;
        strcpy(response.message, "Surrender successfully");
        response.user = *loser;
        response.game = *game;
        send(client_fd, &response, sizeof(response), 0);

        response.type = MSG_SURRENDER;
        strcpy(response.message, "Opponent surrendered");
        response.user = *winner;
        response.game = *game;
        send(game->client2, &response, sizeof(response), 0);

        printf("%d> SURRENDER(): %s surrender\n", client_fd, loser->username);

        clear_game(game);
    } else if (game->client2 == client_fd) {
        user_t* winner = get_user(game->user1.username);
        user_t* loser = get_user(game->user2.username);
        winner->wins++;
        winner->elo += 5;
        loser->losses++;
        update_user(winner);
        update_user(loser);
        response.user = *winner;
        game->status = G_STATUS_PLAYER1_WIN;

        for (int i = 0; i < game->crosswords.num_rows; i++) {
            strcpy(game->crosswords.crossword[i], crosswords[game->crossword_index].crossword[i]);
        }

        response.type = MSG_OK;
        strcpy(response.message, "Surrender successfully");
        response.user = *loser;
        response.game = *game;
        send(client_fd, &response, sizeof(response), 0);

        response.type = MSG_SURRENDER;
        strcpy(response.message, "Opponent surrendered");
        response.user = *winner;
        response.game = *game;
        send(game->client1, &response, sizeof(response), 0);

        printf("%d> SURRENDER(): %s surrender\n", client_fd, loser->username);

        clear_game(game);
    } else {
        strcpy(response.message, "Invalid request");
        send(client_fd, &response, sizeof(response), 0);
        printf("%d> SURRENDER(): %s\n", client_fd, response.message);
    }
}
