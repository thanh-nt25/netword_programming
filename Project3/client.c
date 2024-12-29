#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <curses.h>

#include "struct.h"

#define COLOR_PAIR_DEFAULT              1
#define COLOR_PAIR_BOTTOM_BAR           2
#define COLOR_PAIR_CROSSWORD            3
#define COLOR_PAIR_CROSSWORD_SECRET     4
#define COLOR_PAIR_PLAYER               5

WINDOW* p_bottom_bar;

const user_t EMPTY_USER = {"", "", 0, 0, 0, 0};

int server_fd = -1;

user_t current_user;
int is_logged_in = 0;

game_t current_game;

int init_connection(const char* ip, long port);
void disconnect(int signum);

void start();
void print_welcome_window(WINDOW* window);
void print_bottom_menu(WINDOW* window, char* menus[], int cursor, int button_num);

int login(WINDOW* main, WINDOW* bottom_bar);
void print_login_window(WINDOW* main);
int request_login(user_t* user, char msg[]);

void register_user(WINDOW* main_window, WINDOW* bottom_bar);
void print_register_window(WINDOW* window);
int request_register(user_t* user, char msg[]);

int request_logout(user_t* user, char msg[]);

void main_screen(WINDOW* main_window, WINDOW* bottom_bar);
void print_users(WINDOW* window, user_t* users, int num_users, int cursor, int page);
int request_users(user_t* users, int* num_users, char msg[], message_t* challenge);
int request_challenge(user_t* user, char msg[]);

void challenge_user(WINDOW* main_window, WINDOW* bottom_bar, user_t* user);
void print_challenge_window(WINDOW* window, user_t* user);

void response_challenge(WINDOW* main_window, WINDOW* bottom_bar, user_t* user);
void print_response_challenge_window(WINDOW* window, user_t* user);
int request_response_challenge(user_t* user, message_type_t type, char msg[]);

void run_game(WINDOW* main_window, WINDOW* bottom_bar);
void print_crossword(WINDOW* window, crosswords_t* crosswords);
void print_players(WINDOW* window);

void guess_row(WINDOW* main_window, WINDOW* bottom_bar);
void guess_secret(WINDOW* main_window, WINDOW* bottom_bar);
int request_guess(message_t* request, char msg[]);
int request_surrender(char msg[]);

int main(int argc, char* argv[]) {
    signal(SIGINT, disconnect);

    const char* ip = DEFAULT_IP;
    long port = DEFAULT_PORT;

    if (argc > 2) {
        ip = argv[1];
        port = strtol(argv[2], NULL, 10);
    }

    memset(&current_user, 0x00, sizeof(user_t));
    if (init_connection(ip, port) == -1) {
        perror("Cannot connect to server");
        return EXIT_FAILURE;
    }

    start();

    return EXIT_SUCCESS;
}

int init_connection(const char* ip, long port) {
    struct sockaddr_in server_addr = {0x00};
    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip);
    server_addr.sin_port = htons(port);

    if (connect(server_fd, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1) {
        return -1;
    }

    return 0;
}

void disconnect(int signum) {
    message_t request;
    request.type = MSG_QUIT;
    send(server_fd, &request, sizeof(request), 0);

    if (signum == SIGINT || signum == SIGKILL) {
        endwin();
        exit(EXIT_SUCCESS);
    }

    printf("Disconnected from server\n");
}

void start() {
    current_user = EMPTY_USER;
    current_game.created = -1;

    WINDOW* main_window;
    WINDOW* bottom_bar;

    char* menus[] = {"LOGIN", "REGISTER", "EXIT"};
    int main_cursor = 0, menu_num = 3, is_not_exit = 1, login_res = 0;
    int ch;
    initscr();

    if (has_colors() == FALSE) {
        puts("Terminal does not support colors!");
        endwin();
        return;
    } else {
        start_color();
        init_pair(COLOR_PAIR_DEFAULT, COLOR_BLACK, COLOR_WHITE);
        init_pair(COLOR_PAIR_BOTTOM_BAR, COLOR_WHITE, COLOR_BLUE);
        init_pair(COLOR_PAIR_CROSSWORD, COLOR_WHITE, COLOR_BLUE);
        init_pair(COLOR_PAIR_CROSSWORD_SECRET, COLOR_WHITE, COLOR_GREEN);
        init_pair(COLOR_PAIR_PLAYER, COLOR_RED, COLOR_WHITE);
    }

    refresh();
    main_window = newwin(23, 90, 0, 0);
    bottom_bar = newwin(3, 90, 23, 0);
    p_bottom_bar = bottom_bar;

    wbkgd(main_window, COLOR_PAIR(COLOR_PAIR_DEFAULT));
    wbkgd(bottom_bar, COLOR_PAIR(COLOR_PAIR_BOTTOM_BAR));

    signal(SIGINT, disconnect);
    signal(SIGKILL, disconnect);

    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);

    while (is_not_exit != 0) {
        print_welcome_window(main_window);
        print_bottom_menu(bottom_bar, menus, main_cursor, menu_num);

        ch = getch();

        switch (ch) {
            case KEY_LEFT:
                if (main_cursor != 0) {
                    main_cursor--;
                }
                break;

            case KEY_RIGHT:
                if (main_cursor != (menu_num - 1)) {
                    main_cursor++;
                }
                break;

            case '\n':
                switch (main_cursor) {
                    case 0:
                        login_res = login(main_window, bottom_bar);
                        if (login_res == 1) {
                            main_screen(main_window, bottom_bar);
                        }
                        break;

                    case 1:
                        register_user(main_window, bottom_bar);
                        break;

                    case 2:
                        is_not_exit = 0;
                        break;

                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }

    disconnect(0);
    endwin();
}

void print_welcome_window(WINDOW* window) {
    wclear(window);
    mvwprintw(window, 7, 34, "O Chu Bi Mat");
    wrefresh(window);
}

void print_bottom_menu(WINDOW* window, char* menus[], int cursor, int button_num) {
    wclear(window);

    int y = 1, x = 10;
    if (button_num == 1) {
        x = 39;
    }

    for (int i = 0; i < button_num; ++i) {
        if (cursor == i) {
            wattron(window, A_REVERSE);
            mvwprintw(window, y, x + (80 / (button_num)) * i, "%s", menus[i]);
            wattroff(window, A_REVERSE);
        } else {
            mvwprintw(window, y, x + (80 / (button_num)) * i, "%s", menus[i]);
        }
    }

    wrefresh(window);
}

int login(WINDOW* main_window, WINDOW* bottom_bar) {
    char* menus[] = {"LOGIN", "BACK"};
    user_t* user = (user_t*) malloc(sizeof(user_t));
    char username[255];
    char password[255];
    int menu_num = 2, is_not_back = 1, res = 1, main_cursor = 0;
    int ch;
    char msg[255];
    char str[255];

    wclear(main_window);
    wclear(bottom_bar);
    wrefresh(bottom_bar);
    noecho();


    keypad(stdscr, TRUE);

    while (is_not_back != 0) {
        curs_set(1);

        print_login_window(main_window);
        print_bottom_menu(bottom_bar, menus, main_cursor, menu_num);

        memset(username, 0x00, sizeof(username) / sizeof(char));
        memset(password, 0x00, sizeof(password) / sizeof(char));
        memset(user, 0x00, sizeof(user_t));
        int is_enter = 0, i = 0;
        main_cursor = 0;
        res = 1;

        wmove(main_window, 8, 36);
        while (is_enter == 0) {
            ch = wgetch(main_window);
            switch (ch) {
                case 8:
                case 127:
                    if (i > 0) {
                        username[--i] = 0;
                        wmove(main_window, 8, 36 + i);
                        wechochar(main_window, ' ');
                    }
                    break;

                case '\n':
                    is_enter = 1;
                    break;

                default:
                    username[i++] = (char) ch;
                    break;
            }
            wmove(main_window, 8, 36);
            for (int j = 0; j < i; j++) {
                wechochar(main_window, username[j]);
            }
        }

        is_enter = 0;
        i = 0;
        wmove(main_window, 10, 36);
        noecho();
        while (is_enter == 0) {
            ch = wgetch(main_window);
            switch (ch) {
                case 8:   // Backspace
                case 127: // DEL
                    if (i > 0) {
                        password[--i] = 0;
                        wmove(main_window, 10, 36 + i);
                        wechochar(main_window, ' ');
                    }
                    break;

                case '\n':
                    is_enter = 1;
                    break;

                default:
                    password[i++] = (char) ch;
                    break;
            }

            wmove(main_window, 10, 36);
            for (int j = 0; j < i; j++) {
                wechochar(main_window, '*');
            }
        }

        strcpy(user->username, username);
        strcpy(user->password, password);

        curs_set(0);

        while (is_not_back != 0 && res != 0) {
            print_bottom_menu(bottom_bar, menus, main_cursor, menu_num);

            ch = getch();
            switch (ch) {
                case KEY_LEFT:
                    if (main_cursor != 0) {
                        main_cursor--;
                    }
                    break;

                case KEY_RIGHT:
                    if (main_cursor != (menu_num - 1)) {
                        main_cursor++;
                    }
                    break;

                case '\n':
                    switch (main_cursor) {
                        case 0:
                            res = request_login(user, msg);
                            if (res != 0) {
                                is_logged_in = 1;
                                free(user);
                                return 1;
                            } else {
                                wclear(bottom_bar);
                                memset(str, 0x00, 255);
                                strcpy(str, ">>> ");
                                strcat(str, msg);
                                strcat(str, ". (Press any key...)");
                                mvwprintw(bottom_bar, 1, 0, "%s", str);
                                wrefresh(bottom_bar);
                                getch();
                            }
                            break;

                        case 1:
                            is_not_back = 0;
                            break;

                        default:
                            break;
                    }
                    break;

                default:
                    break;
            }
        }
    }

    free(user);
    return 0;
}

void print_login_window(WINDOW* window) {
    wclear(window);
    mvwprintw(window, 6, 37, "LOGIN");
    mvwprintw(window, 8, 25, "USERNAME: ");
    mvwprintw(window, 10, 25, "PASSWORD: ");
    wrefresh(window);
}

int request_login(user_t* user, char msg[]) {
    message_t request;
    request.type = MSG_LOGIN;
    strcpy(request.user.username, user->username);
    strcpy(request.user.password, user->password);
    send(server_fd, &request, sizeof(request), 0);

    message_t response;
    do {
        recv(server_fd, &response, sizeof(response), 0);
    } while (response.type != MSG_OK && response.type != MSG_ERROR);

    if (response.type == MSG_OK) {
        current_user = response.user;
        return 1;
    } else {
        strcpy(msg, response.message);
        return 0;
    }
}

void register_user(WINDOW* main_window, WINDOW* bottom_bar) {
    char* menus[] = {"REGISTER", "BACK"};
    char username[255];
    char password[255];
    user_t* user = (user_t*) malloc(sizeof(user_t));

    int menu_num = 2, is_not_back = 1, res = 1, main_cursor = 0;
    int ch;
    char msg[255];
    char str[255];

    wrefresh(bottom_bar);

    noecho();
    cbreak();

    keypad(stdscr, TRUE);

    while (is_not_back != 0) {
        curs_set(1);

        print_register_window(main_window);
        print_bottom_menu(bottom_bar, menus, main_cursor, menu_num);

        memset(username, 0x00, sizeof(username) / sizeof(char));
        memset(password, 0x00, sizeof(password) / sizeof(char));
        memset(user, 0x00, sizeof(user_t));
        int is_enter = 0, i = 0;
        main_cursor = 0;
        res = 1;

        wmove(main_window, 8, 36);
        while (is_enter == 0) {
            ch = wgetch(main_window);
            switch (ch) {
                case 8:
                case 127:
                    if (i > 0) {
                        username[--i] = 0;
                        wmove(main_window, 8, 36 + i);
                        wechochar(main_window, ' ');
                    }
                    break;

                case '\n':
                    is_enter = 1;
                    break;

                default:
                    username[i++] = (char) ch;
                    break;
            }
            wmove(main_window, 8, 36);
            for (int j = 0; j < i; j++) {
                wechochar(main_window, username[j]);
            }
        }

        is_enter = 0;
        i = 0;
        wmove(main_window, 10, 36);
        noecho();
        while (is_enter == 0) {
            ch = wgetch(main_window);
            switch (ch) {
                case 8: // Backspace
                case 127:
                    if (i > 0) {
                        password[--i] = 0;
                        wmove(main_window, 10, 36 + i);
                        wechochar(main_window, ' ');
                    }
                    break;

                case '\n':
                    is_enter = 1;
                    break;

                default:
                    password[i++] = (char) ch;
                    break;
            }
            wmove(main_window, 10, 36);
            for (int j = 0; j < i; j++) {
                wechochar(main_window, '*');
            }
        }

        strcpy(user->username, username);
        strcpy(user->password, password);

        curs_set(0);

        while (is_not_back != 0 && res != 0) {
            print_bottom_menu(bottom_bar, menus, main_cursor, menu_num);

            ch = getch();
            switch (ch) {
                case KEY_LEFT:
                    if (main_cursor != 0) {
                        main_cursor--;
                    }
                    break;

                case KEY_RIGHT:
                    if (main_cursor != (menu_num - 1)) {
                        main_cursor++;
                    }
                    break;

                case '\n':
                    switch (main_cursor) {
                        case 0:
                            res = request_register(user, msg);
                            if (res) {
                                wclear(bottom_bar);
                                mvwprintw(bottom_bar, 1, 0, ">>> Welcome to Game! (Press any key...)");
                                wrefresh(bottom_bar);
                                getch();
                                free(user);
                                return;
                            } else {
                                wclear(bottom_bar);
                                memset(str, 0x00, 255);
                                strcpy(str, ">>> ");
                                strcat(str, msg);
                                strcat(str, ". (Press any key...)");
                                mvwprintw(bottom_bar, 1, 0, "%s", str);
                                wrefresh(bottom_bar);
                                getch();
                            }
                            break;

                        case 1:
                            is_not_back = 0;
                            break;

                        default:
                            break;
                    }
                    break;

                default:
                    break;
            }
        }
    }

    free(user);
}

void print_register_window(WINDOW* window) {
    wclear(window);
    mvwprintw(window, 6, 36, "REGISTER");
    mvwprintw(window, 8, 25, "USERNAME : ");
    mvwprintw(window, 10, 25, "PASSWORD : ");
    wrefresh(window);
}

int request_register(user_t* user, char msg[]) {
    message_t request;
    request.type = MSG_REGISTER;
    strcpy(request.user.username, user->username);
    strcpy(request.user.password, user->password);
    send(server_fd, &request, sizeof(request), 0);

    message_t response;
    do {
        recv(server_fd, &response, sizeof(response), 0);
    } while (response.type != MSG_OK && response.type != MSG_ERROR);

    if (response.type == MSG_OK) {
        return 1;
    } else {
        strcpy(msg, response.message);
        return 0;
    }
}

int request_logout(user_t* user, char msg[]) {
    message_t request;
    request.type = MSG_LOGOUT;
    strcpy(request.user.username, user->username);
    strcpy(request.user.password, user->password);
    send(server_fd, &request, sizeof(request), 0);

    message_t response;
    do {
        recv(server_fd, &response, sizeof(response), 0);
    } while (response.type != MSG_OK && response.type != MSG_ERROR);

    if (response.type == MSG_OK) {
        return 1;
    } else {
        strcpy(msg, response.message);
        return 0;
    }
}

void main_screen(WINDOW* main_window, WINDOW* bottom_bar) {
    char* menus[] = {"REFRESH", "PREV", "NEXT", "LOGOUT"};
    int main_cursor = 0, menu_num = 4, page = 0;

    int ch;
    char msg[255];
    char str[255];
    int res;

    message_t challenge;
    user_t users[CLIENTS_SIZE];
    int num_users = 0;

    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);

    request_users(users, &num_users, msg, &challenge);

    while (is_logged_in != 0) {
        if (challenge.type == MSG_CHALLENGE) {
            response_challenge(main_window, bottom_bar, &challenge.user);
            challenge.type = MSG_ERROR;
        }

        int num_users_visible = num_users - page * PAGE_SIZE;
        if (num_users_visible > PAGE_SIZE) {
            num_users_visible = PAGE_SIZE;
        }

        print_users(main_window, users, num_users, main_cursor, page);
        if (main_cursor < num_users_visible) {
            print_bottom_menu(bottom_bar, menus, -1, menu_num);
        } else {
            print_bottom_menu(bottom_bar, menus, main_cursor - num_users_visible, menu_num);
        }

        ch = getch();

        switch (ch) {
            case KEY_LEFT:
            case KEY_UP:
                if (main_cursor != 0) {
                    main_cursor--;
                }
                break;

            case KEY_RIGHT:
            case KEY_DOWN:
                if (main_cursor != (num_users_visible + menu_num - 1)) {
                    main_cursor++;
                }
                break;

            case '\n':
                if (main_cursor >= 0 && main_cursor < num_users_visible) {
                    res = request_challenge(&users[main_cursor + page * PAGE_SIZE], msg);
                    if (res) {
                        challenge_user(main_window, bottom_bar, &users[main_cursor + page * PAGE_SIZE]);
                    } else {
                        wclear(bottom_bar);
                        memset(str, 0x00, 255);
                        strcat(str, ">>> ");
                        strcat(str, msg);
                        strcat(str, ". (Press any key...)");
                        mvwprintw(bottom_bar, 1, 0, "%s", str);
                        wrefresh(bottom_bar);
                        getch();
                    }
                } else if (main_cursor == num_users_visible) {
                    res = request_users(users, &num_users, msg, &challenge);
                    if (res) {
                        main_cursor = 0;
                        page = 0;
                        // wclear(bottom_bar);
                        // memset(str, 0x00, 255);
                        // strcat(str, ">> Get success. (Press any key...)");
                        // mvwprintw(bottom_bar, 1, 0, "%s", str);
                        // wrefresh(bottom_bar);
                        // getch();
                    } else {
                        wclear(bottom_bar);
                        memset(str, 0x00, 255);
                        strcat(str, ">>> ");
                        strcat(str, msg);
                        strcat(str, ". (Press any key...)");
                        mvwprintw(bottom_bar, 1, 0, "%s", str);
                        wrefresh(bottom_bar);
                        getch();
                    }
                } else if (main_cursor == (num_users_visible + 1)) {
                    if (page > 0) {
                        page--;
                        res = request_users(users, &num_users, msg, &challenge);
                        if (res == 0) {
                            wclear(bottom_bar);
                            memset(str, 0x00, 255);
                            strcat(str, ">>> ");
                            strcat(str, msg);
                            strcat(str, ". (Press any key...)");
                            mvwprintw(bottom_bar, 1, 0, "%s", str);
                            wrefresh(bottom_bar);
                            getch();
                        } else {
                            main_cursor = 0;
                        }
                    }
                } else if (main_cursor == (num_users_visible + 2)) {
                    if (page < (num_users / PAGE_SIZE)) {
                        page++;
                        res = request_users(users, &num_users, msg, &challenge);
                        if (res == 0) {
                            wclear(bottom_bar);
                            memset(str, 0x00, 255);
                            strcat(str, ">>> ");
                            strcat(str, msg);
                            strcat(str, ". (Press any key...)");
                            mvwprintw(bottom_bar, 1, 0, "%s", str);
                            wrefresh(bottom_bar);
                            getch();
                        } else {
                            main_cursor = 0;
                        }
                    }
                } else if (main_cursor == (num_users_visible + 3)) {
                    res = request_logout(&current_user, msg);
                    if (res) {
                        is_logged_in = 0;
                        return;
                    } else {
                        wclear(bottom_bar);
                        memset(str, 0x00, 255);
                        strcat(str, ">>> ");
                        strcat(str, msg);
                        strcat(str, ". (Press any key...)");
                        mvwprintw(bottom_bar, 1, 0, "%s", str);
                        wrefresh(bottom_bar);
                        getch();
                    }
                }
                break;

            default:
                break;
        }
    }
}

void print_users(WINDOW* window, user_t* users, int num_users, int cursor, int page) {
    wclear(window);
    mvwprintw(window, 0, 0, "USERS (Page %d)", page + 1);
    mvwprintw(window, 1, 0, "----------------------------------------------------");
    mvwprintw(window, 2, 0, "No. |   Username   | Wins | Draws | Losses |  ELO  |");
    mvwprintw(window, 3, 0, "----------------------------------------------------");

    int start = page * PAGE_SIZE;
    int end = start + PAGE_SIZE;
    if (end > num_users) {
        end = num_users;
    }

    for (int i = start; i < end && i < num_users; i++) {
        if (cursor == i) {
            wattron(window, A_REVERSE);
            mvwprintw(window, 4 + i - start, 0, "%3d | %-12s | %4d | %5d | %6d | %5d |", i + 1, users[i].username, users[i].wins, users[i].draws, users[i].losses, users[i].elo);
            wattroff(window, A_REVERSE);
        } else {
            mvwprintw(window, 4 + i - start, 0, "%3d | %-12s | %4d | %5d | %6d | %5d |", i + 1, users[i].username, users[i].wins, users[i].draws, users[i].losses, users[i].elo);
        }
    }

    wrefresh(window);
}

int request_users(user_t* users, int* num_users, char msg[], message_t* challenge) {
    message_t request;
    request.type = MSG_GET_USERS;
    strcpy(request.user.username, current_user.username);
    send(server_fd, &request, sizeof(request), 0);

    message_t response;
    do {
        recv(server_fd, &response, sizeof(response), 0);

        if (response.type == MSG_CHALLENGE) {
            *challenge = response;
        }
    } while (response.type != MSG_OK && response.type != MSG_ERROR);

    int result = 0, num_users_tmp = 0;
    if (response.type == MSG_OK) {
        recv(server_fd, &num_users_tmp, sizeof(int), 0);
        for (int i = 0; i < num_users_tmp; i++) {
            recv(server_fd, &users[i], sizeof(user_t), 0);
        }
        result = 1;
        *num_users = num_users_tmp;
    } else {
        strcpy(msg, response.message);
        result = 0;
    }

    return result;
}

int request_challenge(user_t* user, char msg[]) {
    message_t request;
    request.type = MSG_CHALLENGE;
    strcpy(request.user.username, user->username);
    send(server_fd, &request, sizeof(request), 0);

    message_t response;
    do {
        recv(server_fd, &response, sizeof(response), 0);
    } while (response.type != MSG_OK && response.type != MSG_ERROR);

    if (response.type == MSG_OK) {
        return 1;
    } else {
        strcpy(msg, response.message);
        return 0;
    }
}

void challenge_user(WINDOW* main_window, WINDOW* bottom_bar, user_t* user) {
    print_challenge_window(main_window, user);
    wclear(bottom_bar);
    wrefresh(bottom_bar);

    message_t response;
    do {
        recv(server_fd, &response, sizeof(response), 0);
    } while (response.type != MSG_ACCEPT && response.type != MSG_DECLINE);

    if (response.type == MSG_ACCEPT) {
        current_game = response.game;
        run_game(main_window, bottom_bar);
    } else {
        wclear(bottom_bar);
        mvwprintw(bottom_bar, 1, 0, ">>> %s declined your challenge. (Press any key...)", user->username);
        wrefresh(bottom_bar);
        getch();
    }
}

void print_challenge_window(WINDOW* window, user_t* user) {
    wclear(window);
    mvwprintw(window, 6, 36, "CHALLENGE");
    mvwprintw(window, 8, 25, "Waiting for %s response", user->username);
    wrefresh(window);
}

void response_challenge(WINDOW* main_window, WINDOW* bottom_bar, user_t* user) {
    char* menus[] = {"ACCEPT", "DECLINE"};
    int main_cursor = 0, menu_num = 2;
    int ch;
    char msg[255];
    char str[255];
    int res;

    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);

    int is_running = 1;
    while (is_running != 0) {
        print_response_challenge_window(main_window, user);
        print_bottom_menu(bottom_bar, menus, main_cursor, menu_num);

        ch = getch();

        switch (ch) {
            case KEY_LEFT:
                if (main_cursor != 0) {
                    main_cursor--;
                }
                break;

            case KEY_RIGHT:
                if (main_cursor != (menu_num - 1)) {
                    main_cursor++;
                }
                break;

            case '\n':
                switch (main_cursor) {
                    case 0:
                        res = request_response_challenge(user, MSG_ACCEPT, msg);
                        if (res) {
                            run_game(main_window, bottom_bar);
                            is_running = 0;
                        } else {
                            wclear(bottom_bar);
                            memset(str, 0x00, 255);
                            strcat(str, ">>> ");
                            strcat(str, msg);
                            strcat(str, ". (Press any key...)");
                            mvwprintw(bottom_bar, 1, 0, "%s", str);
                            wrefresh(bottom_bar);
                            getch();
                        }
                        break;

                    case 1:
                        res = request_response_challenge(user, MSG_DECLINE, msg);
                        if (res) {
                            wclear(bottom_bar);
                            mvwprintw(bottom_bar, 1, 0, ">>> Declined %s challenge. (Press any key...)", user->username);
                            wrefresh(bottom_bar);
                            getch();
                            is_running = 0;
                        } else {
                            wclear(bottom_bar);
                            memset(str, 0x00, 255);
                            strcat(str, ">>> ");
                            strcat(str, msg);
                            strcat(str, ". (Press any key...)");
                            mvwprintw(bottom_bar, 1, 0, "%s", str);
                            wrefresh(bottom_bar);
                            getch();
                        }
                        break;

                    default:
                        break;
                }
                break;

            default:
                break;
        }
    }
}

void print_response_challenge_window(WINDOW* window, user_t* user) {
    wclear(window);
    mvwprintw(window, 6, 36, "CHALLENGE");
    mvwprintw(window, 8, 25, "%s wants to challenge you", user->username);
    wrefresh(window);
}

int request_response_challenge(user_t* user, message_type_t type, char msg[]) {
    message_t request;
    request.type = type;
    strcpy(request.user.username, user->username);
    send(server_fd, &request, sizeof(request), 0);

    message_t response;
    do {
        recv(server_fd, &response, sizeof(response), 0);
    } while (response.type != MSG_OK && response.type != MSG_ERROR);

    if (response.type == MSG_OK) {
        if (type == MSG_ACCEPT) {
            current_game = response.game;
        }
        return 1;
    } else {
        strcpy(msg, response.message);
        return 0;
    }
}

void run_game(WINDOW* main_window, WINDOW* bottom_bar) {
    char* menus[] = {"GUESS ROW", "GUESS SECRET", "SURRENDER"};
    int main_cursor = 0, menu_num = 3;

    int ch;
    char msg[255];
    char str[255];
    int res;

    wclear(main_window);
    wclear(bottom_bar);
    wrefresh(main_window);
    wrefresh(bottom_bar);
    noecho();

    keypad(stdscr, TRUE);

    while (current_game.status == G_STATUS_RUNNING) {
        wclear(main_window);
        print_crossword(main_window, &current_game.crosswords);
        print_players(main_window);
        print_bottom_menu(bottom_bar, menus, main_cursor, menu_num);

        if ((current_game.turn == 0 && strcmp(current_user.username, current_game.user1.username) == 0) ||
            (current_game.turn == 1 && strcmp(current_user.username, current_game.user2.username) == 0)) {
            int is_running = 1;
            while (is_running != 0) {
                print_bottom_menu(bottom_bar, menus, main_cursor, menu_num);

                ch = getch();

                switch (ch) {
                    case KEY_LEFT:
                        if (main_cursor != 0) {
                            main_cursor--;
                        }
                        break;

                    case KEY_RIGHT:
                        if (main_cursor != (menu_num - 1)) {
                            main_cursor++;
                        }
                        break;

                    case '\n':
                        switch (main_cursor) {
                            case 0:
                                guess_row(main_window, bottom_bar);
                                is_running = 0;
                                break;

                            case 1:
                                guess_secret(main_window, bottom_bar);
                                is_running = 0;
                                break;

                            case 2:
                                res = request_surrender(msg);
                                if (res) {
                                    wclear(bottom_bar);
                                    mvwprintw(bottom_bar, 1, 0, ">>> You surrendered. (Press any key...)");
                                    wrefresh(bottom_bar);
                                    getch();
                                    return;
                                } else {
                                    wclear(bottom_bar);
                                    memset(str, 0x00, 255);
                                    strcat(str, ">>> ");
                                    strcat(str, msg);
                                    strcat(str, ". (Press any key...)");
                                    mvwprintw(bottom_bar, 1, 0, "%s", str);
                                    wrefresh(bottom_bar);
                                    getch();
                                }
                                break;

                            default:
                                break;
                        }
                        break;

                    default:
                        break;
                }
            }
        } else {
            wclear(bottom_bar);
            mvwprintw(bottom_bar, 1, 0, ">>> Waiting for opponent...");
            wrefresh(bottom_bar);

            message_t response;
            do {
                recv(server_fd, &response, sizeof(response), 0);
            } while (response.type != MSG_OK && response.type != MSG_ERROR && response.type != MSG_SURRENDER);

            if (response.type == MSG_OK) {
                current_game = response.game;
            } else if (response.type == MSG_ERROR) {
                wclear(bottom_bar);
                memset(str, 0x00, 255);
                strcat(str, ">>> ");
                strcat(str, response.message);
                strcat(str, ". (Press any key...)");
                mvwprintw(bottom_bar, 1, 0, "%s", str);
                wrefresh(bottom_bar);
                getch();
            } else if (response.type == MSG_SURRENDER) {
                current_game = response.game;
                wclear(bottom_bar);
                mvwprintw(bottom_bar, 1, 0, ">>> Opponent surrendered. (Press any key...)");
                wrefresh(bottom_bar);
                getch();
            }
        }
    }

    print_crossword(main_window, &current_game.crosswords);
    print_players(main_window);
    wclear(bottom_bar);
    if (current_game.status == G_STATUS_PLAYER1_WIN) {
        if (strcmp(current_user.username, current_game.user1.username) == 0) {
            mvwprintw(bottom_bar, 1, 0, ">>> You win! (Press any key...)");
        } else {
            mvwprintw(bottom_bar, 1, 0, ">>> You lose! (Press any key...)");
        }
        wrefresh(bottom_bar);
        getch();
    } else if (current_game.status == G_STATUS_PLAYER2_WIN) {
        if (strcmp(current_user.username, current_game.user2.username) == 0) {
            mvwprintw(bottom_bar, 1, 0, ">>> You win! (Press any key...)");
        } else {
            mvwprintw(bottom_bar, 1, 0, ">>> You lose! (Press any key...)");
        }
        wrefresh(bottom_bar);
        getch();
    } else {
        mvwprintw(bottom_bar, 1, 0, ">>> Draw! (Press any key...)");
        wrefresh(bottom_bar);
        getch();
    }
}

void print_crossword(WINDOW* window, crosswords_t* crosswords) {
    if (crosswords == NULL) {
        return;
    }

    int prev_start_col = -1, prev_len = -1, row;
    for (row = 0; row < crosswords->num_rows; row++) {
        mvwprintw(window, 2 * row + 1, 0, "%2d:", row + 1);
        int start_col = 3 + crosswords->start_cols[row] * 2;
        int len = strlen(crosswords->crossword[row]);
        for (int i = 0; i < len; i++) {
            int y = 2 * row;
            int x = 2 * i + start_col;

            if (row == 0) {
                if (i == 0) {
                    mvwaddch(window, 0, x, ACS_ULCORNER);
                } else {
                    mvwaddch(window, 0, x, ACS_TTEE);
                    if (i == len - 1) {
                        mvwaddch(window, 0, x + 2, ACS_URCORNER);
                    }
                }
            } else {
                if (i == 0) {
                    if (prev_start_col > start_col) {
                        mvwaddch(window, y, start_col, ACS_ULCORNER);
                    } else if (prev_start_col == start_col) {
                        mvwaddch(window, y, start_col, ACS_LTEE);
                    } else {
                        mvwaddch(window, y, start_col, ACS_PLUS);
                    }
                } else {
                    if (x >= prev_start_col && x <= prev_start_col + prev_len * 2) {
                        mvwaddch(window, y, x, ACS_PLUS);
                    } else {
                        mvwaddch(window, y, x, ACS_TTEE);
                    }
                    if (i == len - 1) {
                        if (prev_start_col + prev_len * 2 > start_col + len * 2) {
                            mvwaddch(window, y, x + 2, ACS_PLUS);
                        } else if (prev_start_col + prev_len * 2 == start_col + len * 2) {
                            mvwaddch(window, y, x + 2, ACS_RTEE);
                        } else {
                            mvwaddch(window, y, x + 2, ACS_URCORNER);
                        }
                    }
                }
            }

            mvwaddch(window, y, x + 1, ACS_HLINE);
            mvwaddch(window, y + 1, x, ACS_VLINE);
            mvwaddch(window, y + 1, x + 2, ACS_VLINE);

            if (crosswords->secret_col == i + crosswords->start_cols[row]) {
                wattron(window, COLOR_PAIR(COLOR_PAIR_CROSSWORD_SECRET));
            }
            mvwprintw(window, 2 * row + 1, start_col + 2 * i + 1, "%c", crosswords->crossword[row][i]);
            if (crosswords->secret_col == i + crosswords->start_cols[row]) {
                wattroff(window, COLOR_PAIR(COLOR_PAIR_CROSSWORD_SECRET));
            }
        }

        if (prev_start_col != -1 && prev_start_col <= start_col) {
            for (int i = prev_start_col; i < start_col; i += 2) {
                if (i == prev_start_col) {
                    mvwaddch(window, 2 * row, i, ACS_LLCORNER);
                } else {
                    mvwaddch(window, 2 * row, i, ACS_BTEE);
                }
                mvwaddch(window, 2 * row, i + 1, ACS_HLINE);
            }
        }

        if (prev_start_col != -1 && prev_start_col + prev_len * 2 >= start_col + len * 2) {
            for (int i = start_col + len * 2; i < prev_start_col + prev_len * 2; i += 2) {
                mvwaddch(window, 2 * row, i + 1, ACS_HLINE);
                if (i + 2 == prev_start_col + prev_len * 2) {
                    mvwaddch(window, 2 * row, i + 2, ACS_LRCORNER);
                } else {
                    mvwaddch(window, 2 * row, i + 2, ACS_BTEE);
                }
            }
        }

        prev_start_col = start_col;
        prev_len = len;
    }

    for (int i = 0; i < prev_len; i++) {
        int y = 2 * row;
        int x = 2 * i + prev_start_col;

        if (i == 0) {
            mvwaddch(window, y, x, ACS_LLCORNER);
        } else {
            if (i == prev_len - 1) {
                mvwaddch(window, y, x + 2, ACS_LRCORNER);
            }
            mvwaddch(window, y, x, ACS_BTEE);
        }
        mvwaddch(window, y, x + 1, ACS_HLINE);
    }

    wrefresh(window);
}

void print_players(WINDOW* window) {
    if (strcmp(current_user.username, current_game.user1.username) == 0) {
        wattron(window, COLOR_PAIR(COLOR_PAIR_PLAYER));
        mvwprintw(window, 5, 60, "Player 1: %s", current_game.user1.username);
        mvwprintw(window, 6, 60, "Wins: %d", current_game.user1.wins);
        mvwprintw(window, 7, 60, "Draws: %d", current_game.user1.draws);
        mvwprintw(window, 8, 60, "Losses: %d", current_game.user1.losses);
        mvwprintw(window, 9, 60, "ELO: %d", current_game.user1.elo);
        wattroff(window, COLOR_PAIR(COLOR_PAIR_PLAYER));

        mvwprintw(window, 15, 60, "Player 2: %s", current_game.user2.username);
        mvwprintw(window, 16, 60, "Wins: %d", current_game.user2.wins);
        mvwprintw(window, 17, 60, "Draws: %d", current_game.user2.draws);
        mvwprintw(window, 18, 60, "Losses: %d", current_game.user2.losses);
        mvwprintw(window, 19, 60, "ELO: %d", current_game.user2.elo);
    } else {
        mvwprintw(window, 5, 60, "Player 1: %s", current_game.user1.username);
        mvwprintw(window, 6, 60, "Wins: %d", current_game.user1.wins);
        mvwprintw(window, 7, 60, "Draws: %d", current_game.user1.draws);
        mvwprintw(window, 8, 60, "Losses: %d", current_game.user1.losses);
        mvwprintw(window, 9, 60, "ELO: %d", current_game.user1.elo);

        wattron(window, COLOR_PAIR(COLOR_PAIR_PLAYER));
        mvwprintw(window, 15, 60, "Player 2: %s", current_game.user2.username);
        mvwprintw(window, 16, 60, "Wins: %d", current_game.user2.wins);
        mvwprintw(window, 17, 60, "Draws: %d", current_game.user2.draws);
        mvwprintw(window, 18, 60, "Losses: %d", current_game.user2.losses);
        mvwprintw(window, 19, 60, "ELO: %d", current_game.user2.elo);
        wattroff(window, COLOR_PAIR(COLOR_PAIR_PLAYER));
    }

    wrefresh(window);
}

void guess_row(WINDOW* main_window, WINDOW* bottom_bar) {
    char input[32];
    int ch;
    char msg[255];
    char str[255];
    int res;
 
    wclear(main_window);
    wclear(bottom_bar);
    wrefresh(main_window);
    wrefresh(bottom_bar);
    noecho();
 
    keypad(stdscr, TRUE);
 
    print_crossword(main_window, &current_game.crosswords);
    print_players(main_window);
 
    int is_not_done = 1;
    while (is_not_done) {
        int row = -1;
 
        memset(input, 0x00, 32);
        int i = 0;
        int is_enter = 0;
 
        wclear(bottom_bar);
        mvwprintw(bottom_bar, 1, 0, "Enter row guess: ");
        wrefresh(bottom_bar);
        wmove(bottom_bar, 1, 20);
        while (is_enter == 0) {
            ch = wgetch(bottom_bar);
            switch (ch) {
                case 8:
                case 127:
                    if (i > 0) {
                        input[--i] = 0;
                        wmove(bottom_bar, 1, 20 + i);
                        wechochar(bottom_bar, ' ');
                    }
                    break;
 
                case '\n':
                    if (strlen(input) == 0) {
                        break;
                    }
 
                    row = atoi(input);
                    if (row < 1 || row > current_game.crosswords.num_rows || current_game.crosswords.crossword[row - 1][0] != ' ') {
                        wclear(bottom_bar);
                        mvwprintw(bottom_bar, 1, 0, ">>> Invalid row number. (Press any key...)");
                        wrefresh(bottom_bar);
                        getch();
                        break;
                    } else {
                        is_enter = 1;
                    }
                    break;
 
                default:
                    input[i++] = (char) ch;
                    break;
            }
 
            wclear(bottom_bar);
            mvwprintw(bottom_bar, 1, 0, "Enter row guess: ");
            wrefresh(bottom_bar);
 
            wmove(bottom_bar, 1, 20);
            for (int j = 0; j < i; j++) {
                wechochar(bottom_bar, input[j]);
            }
        }
 
        mvwprintw(main_window, 21, 0, "%s", current_game.crosswords.suggestions[row - 1]);
        wrefresh(main_window);
 
        wclear(bottom_bar);
        mvwprintw(bottom_bar, 1, 0, "Enter your guess: ");
        wrefresh(bottom_bar);
        wmove(bottom_bar, 1, 20);
        is_enter = 0;
        i = 0;
        memset(input, 0x00, 32);
        while (is_enter == 0) {
            ch = wgetch(bottom_bar);
            switch (ch) {
                case 8:
                case 127:
                    if (i > 0) {
                        input[--i] = 0;
                        wmove(bottom_bar, 1, 20 + i);
                        wechochar(bottom_bar, ' ');
                    }
                    break;
 
                case '\n':
                    is_enter = 1;
                    break;
 
                default:
                    input[i++] = (char) ch;
                    break;
            }
 
            wmove(bottom_bar, 1, 20);
            for (int j = 0; j < i; j++) {
                wechochar(bottom_bar, input[j]);
            }
        }
 
        message_t request;
        request.type = MSG_GUESS;
        guess_t guess;
        guess.type = GUESS_ROW;
        guess.row = row - 1;
        strcpy(guess.crossword, input);
        strcpy(request.user.username, current_user.username);
        request.guess = guess;
 
        res = request_guess(&request, msg);
        if (res) {
            if (res == 1) {
                is_not_done = 0;
                wclear(bottom_bar);
                print_crossword(main_window, &current_game.crosswords);
                mvwprintw(bottom_bar, 1, 0, ">>> Your guess is correct. (Press any key...)");
                wrefresh(bottom_bar);
                getch();
            } else {
                wclear(bottom_bar);
                print_crossword(main_window, &current_game.crosswords);
                mvwprintw(bottom_bar, 1, 0, ">>> Your guess is incorrect. (Press any key...)");
                wrefresh(bottom_bar);
                getch();
                return;
            }
        } else {
            wclear(bottom_bar);
            memset(str, 0x00, 255);
            strcat(str, ">>> ");
            strcat(str, msg);
            strcat(str, ". (Press any key...)");
            mvwprintw(bottom_bar, 1, 0, "%s", str);
            wrefresh(bottom_bar);
            getch();
        }
        wclear(bottom_bar);
        mvwprintw(bottom_bar, 1, 0, "Press 1 for next row, 2 for secret guess: ");
        wrefresh(bottom_bar);
        int action = wgetch(bottom_bar) - '0';
 
        if (action == 1) {
            // Tiếp tục đoán hàng
            continue;
        } else if (action == 2) {
            // Chuyển sang đoán bí mật
            guess_secret(main_window, bottom_bar);
            break;
        } else {
            // Nếu không phải lựa chọn hợp lệ, tiếp tục vòng lặp
            mvwprintw(bottom_bar, 1, 0, "Invalid option. Please choose again. (Press any key...)");
            wrefresh(bottom_bar);
            getch();
        }
    }
 
    // guess_secret(main_window, bottom_bar);
   
}

void guess_secret(WINDOW* main_window, WINDOW* bottom_bar) {
    char input[32];
    int ch;
    char msg[255];
    char str[255];
    int res;

    wclear(main_window);
    wclear(bottom_bar);
    wrefresh(main_window);
    wrefresh(bottom_bar);
    noecho();

    keypad(stdscr, TRUE);

    print_crossword(main_window, &current_game.crosswords);
    print_players(main_window);

    int is_not_done = 1;
    while (is_not_done) {
        memset(input, 0x00, 32);
        int i = 0;
        int is_enter = 0;

        wclear(bottom_bar);
        mvwprintw(bottom_bar, 1, 0, "Enter secret guess: ");
        wrefresh(bottom_bar);
        wmove(bottom_bar, 1, 20);
        while (is_enter == 0) {
            ch = wgetch(bottom_bar);
            switch (ch) {
                case 8:
                case 127:
                    if (i > 0) {
                        input[--i] = 0;
                        wmove(bottom_bar, 1, 20 + i);
                        wechochar(bottom_bar, ' ');
                    }
                    break;

                case '\n':
                    is_enter = 1;
                    break;

                default:
                    input[i++] = (char) ch;
                    break;
            }

            wclear(bottom_bar);
            mvwprintw(bottom_bar, 1, 0, "Enter secret guess: ");
            wrefresh(bottom_bar);

            wmove(bottom_bar, 1, 20);
            for (int j = 0; j < i; j++) {
                wechochar(bottom_bar, input[j]);
            }
        }

        message_t request;
        request.type = MSG_GUESS;
        guess_t guess;
        guess.type = GUESS_COL;
        strcpy(guess.crossword, input);
        strcpy(request.user.username, current_user.username);
        request.guess = guess;

        res = request_guess(&request, msg);
        if (res) {
            if (res == 1) {
                is_not_done = 0;
                wclear(bottom_bar);
                mvwprintw(bottom_bar, 1, 0, ">>> Your guess is correct. (Press any key...)");
                wrefresh(bottom_bar);
                getch();
            } else {
                wclear(bottom_bar);
                mvwprintw(bottom_bar, 1, 0, ">>> Your guess is incorrect. (Press any key...)");
                wrefresh(bottom_bar);
                getch();
                return;
            }
        } else {
            wclear(bottom_bar);
            memset(str, 0x00, 255);
            strcat(str, ">>> ");
            strcat(str, msg);
            strcat(str, ". (Press any key...)");
            mvwprintw(bottom_bar, 1, 0, "%s", str);
            wrefresh(bottom_bar);
            getch();
        }
    }
}

int request_guess(message_t* request, char msg[]) {
    send(server_fd, request, sizeof(message_t), 0);

    message_t response;
    do {
        recv(server_fd, &response, sizeof(response), 0);
    } while (response.type != MSG_OK && response.type != MSG_ERROR);

    if (response.type == MSG_OK) {
        current_game = response.game;
        return 1;
    } else {
        strcpy(msg, response.message);

        if (strcmp(response.message, "Incorrect guess") == 0) {
            current_game = response.game;
            return 2;
        }

        return 0;
    }
}

int request_surrender(char msg[]) {
    message_t request;
    request.type = MSG_SURRENDER;
    strcpy(request.user.username, current_user.username);
    send(server_fd, &request, sizeof(request), 0);

    message_t response;
    do {
        recv(server_fd, &response, sizeof(response), 0);
    } while (response.type != MSG_OK && response.type != MSG_ERROR);

    if (response.type == MSG_OK) {
        current_game = response.game;
        return 1;
    } else {
        strcpy(msg, response.message);
        return 0;
    }
}

